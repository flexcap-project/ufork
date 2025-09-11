#ifdef __CHERI_PURE_CAPABILITY__

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <stddef.h>
#include "cheri_helpers.h"

#include "mallocmap.h"


/*
Open addressing hash table with 2^n table size

Quadratic probing is used in case of hash collision

Tab indices and hash are size_t

Only the key's address is used for calculating the hash - the
 capability is simply preserved for validation against the provided
 capability once the entry is found.

Lazy deletion is used to avoid overly expensive deletions.

There are some special values that can be used for key:
 - When an element is not present (never been inserted or deleted),
    NOT_PRESENT (= NULL) is used.
 - When an element has been deleted, the key is preserved but the
    value is set to NOT_PRESENT (= NULL).

// TODO: Potentially use Robin Hood hashing for better performance.
*/

#define NOT_PRESENT ((void *) NULL)

#define MINSIZE 8
#define MAXSIZE ((size_t)-1/2 + 1)

inline static size_t mallocmap_keyhash_impl(ptraddr_t k)
{
	size_t r;
#if defined(__aarch64__) && defined(__ARM_FEATURE_CRC32)
	unsigned x = 42;
	__asm__ volatile(
		"crc32cx %w0, %w1, %x2" : "=r"(r) : "r"(x), "r"(k)
	);
#elif defined(__x86_64__)
	r = 42;
	__asm__ volatile(
		"crc32 %1, %0" : "+r"(r) : "r"(k)
	);
#else
	// A very simple "hash" function: the value of 'k' can be used as a key.
	r = k & 0xfffffffful;
#endif
	return r;
}

inline static size_t mallocmap_keyhash(void *k) {
	return mallocmap_keyhash_impl(__builtin_cheri_address_get(k));
}

static int mallocmap_resize(size_t nel, struct __mallocmap_tab *htab)
{
	size_t newsize;
	size_t i, j;
	MALLOCMAP_ENTRY *e, *newe;
	MALLOCMAP_ENTRY *oldtab = htab->entries;
	MALLOCMAP_ENTRY *oldend = htab->entries + htab->mask + 1;

	if (nel > MAXSIZE) nel = MAXSIZE;

	// Ensure newsize is a power of 2, and greater than MINSIZE.
	for (newsize = MINSIZE; newsize < nel; newsize *= 2);

	// Reset tombstones as we are rebuilding the hash table
	htab->tombs = 0;

	// Round up new size to nearest 4096 for mmap.
	size_t map_size = ((newsize * (sizeof *htab->entries)) + 4096 - 1) & -4096;

	htab->entries = mmap(0, map_size,
		PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);

	if (htab->entries == MAP_FAILED) {
		htab->entries = oldtab;
		return 0;
	}

#ifdef __CHERI_PURE_CAPABILITY__
	htab->entries = __builtin_cheri_bounds_set(htab->entries, map_size);
	htab->entries = __builtin_cheri_perms_and(htab->entries, MUSL_CAP_PROT_MALLOC);
#endif

	htab->mask = newsize - 1;

	// If this is the initial setup, return now.
	if (!oldtab) return 1;

	// Now that our table has been resized, we must repopulate it
	//  with new hashes.
	// TODO: Avoidable with prime number magic?
	for (e = oldtab; e < oldend; e++) {
		if (e->key != NOT_PRESENT && e->data != NOT_PRESENT) {
			// Insert into new hashmap
			for (i = mallocmap_keyhash(e->key), j=1; ; i += j++) {
				newe = htab->entries + (i & htab->mask);
				if (newe->key == NOT_PRESENT)
					break;
			}

			newe->key = e->key;
			newe->data = e->data;
		}
	}

	if (oldtab) {
		// Now that the old data has been copied into the new hash table, we
		//  can unmap the old table.
		map_size = ((oldend - oldtab) + 4096 - 1) & -4096;
		munmap(oldtab, map_size);
	}

	return 1;
}

int mallocmap_create(size_t nel, struct __mallocmap_tab *htab)
{
	int r = mallocmap_resize(nel, htab);

	return r;
}

/*
 * Returns a value for the given key.
 *
 * Returns NULL if no value exists.
 */
void *mallocmap_find(void *key, struct __mallocmap_tab *htab) {
	size_t hash = mallocmap_keyhash(key);

	MALLOCMAP_ENTRY *e;
	MALLOCMAP_ENTRY *tomb = 0;
	size_t probe = 1;

	while ((e = htab->entries + (hash & htab->mask))->key != key) {
		if (e->key == NOT_PRESENT) {
			return NULL;
		}

		if (!tomb && e->data == NOT_PRESENT) {
			// Remember this tombstone so we can shift it with our found value.
			tomb = e;
		}

		// Quadratic probing.
		hash += probe++;
	}

	if (tomb) {
		// Overwrite the tomb to improve time on any future searches
		tomb->key = key;
		tomb->data = e->data;

		e->data = NULL;

		e = tomb;
	}

	if(!__builtin_cheri_equal_exact(key, e->key)) {
		// The provided capability is different to when it was inserted.
		return NULL;
	}

	return e->data;
}

/*
 * Inserts a new value.
 *
 * Returns 1 on success, 0 on failure.
 */
int mallocmap_insert(void *key, void *data, struct __mallocmap_tab *htab) {
	size_t hash = mallocmap_keyhash(key);

	MALLOCMAP_ENTRY *e;
	size_t probe = 1;

	while ((e = htab->entries + (hash & htab->mask))) {
		if (e->key == NOT_PRESENT) {
			// New insertion
			break;
		}

		if (e->data == NOT_PRESENT) {
			// Replace tombstone
			htab->tombs--;

			break;
		}

		if (e->key == key) {
			// Key already exists in map
			return 0;
		}

		// Quadratic probing
		hash += probe++;
	}

	// Insert new entry into hash table.
	e->key = key;
	e->data = data;

	// Double hashmap size if our hashmap is too full now.
	if (++htab->used > htab->mask - htab->mask/4) {
		if (!mallocmap_resize(2 * htab->used, htab)) {
			// Resize failed, return error.
			htab->used--;
			e->key = NOT_PRESENT;

			return 0;
		}
	}

	return 1;
}

/*
 * Delete item from hashmap.
 *
 * Returns 1 on success, 0 on failure.
 */
int mallocmap_delete(void *key, struct __mallocmap_tab *htab) {
	size_t hash = mallocmap_keyhash(key);

	MALLOCMAP_ENTRY *e;

	size_t probe = 1;
	size_t index = hash;
	while ((e = htab->entries + (index & htab->mask))->key != key) {
		if (e->key == NOT_PRESENT) {
			return 0;
		}

		// Quadratic probing
		index += probe++;
	}

	e->data = NOT_PRESENT;

	htab->used--;
	if (++htab->tombs > htab->used && htab->used > htab->mask / 4) {
		// Rebuild the hash table if the number of tombs exceeds number of
		//  active items.
		if (!mallocmap_resize(2 * htab->used, htab)) {
			return 0;
		}
	}

	return 1;
}

#endif
