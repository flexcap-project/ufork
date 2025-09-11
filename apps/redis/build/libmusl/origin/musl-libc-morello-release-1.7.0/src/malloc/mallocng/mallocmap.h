#ifndef MALLOC_MAP_H
#define MALLOC_MAP_H

#include <inttypes.h>

// Maps user pointers to widened capabilities.
//  We could use ptraddr_t for key, but this would make us unable
//  to validate the capabilities on the user pointer.
typedef struct mallocmap_entry {
	void *key;
	void *data;
} MALLOCMAP_ENTRY;

struct __mallocmap_tab {
	// The mask that is &ed with the hash to bring it into the
	//  appropriate range to be used as an index into the table
	//  (size - 1).
	size_t mask;
	// Number of active entries in the table.
	size_t used;
	// Number of tombstones (deleted entries) in the table.
	size_t tombs;

	MALLOCMAP_ENTRY *entries;
};

int mallocmap_create(size_t nel, struct __mallocmap_tab *htab);
void *mallocmap_find(void *key, struct __mallocmap_tab *htab);
int mallocmap_insert(void *key, void *data, struct __mallocmap_tab *htab);
int mallocmap_delete(void *key, struct __mallocmap_tab *htab);

#endif