/*
FUNCTION
        <<memcpy>>---copy memory regions

ANSI_SYNOPSIS
        #include <string.h>
        void* memcpy(void *restrict <[out]>, const void *restrict <[in]>,
                     size_t <[n]>);

TRAD_SYNOPSIS
        #include <string.h>
        void *memcpy(<[out]>, <[in]>, <[n]>
        void *<[out]>;
        void *<[in]>;
        size_t <[n]>;

DESCRIPTION
        This function copies <[n]> bytes from the memory region
        pointed to by <[in]> to the memory region pointed to by
        <[out]>.

        If the regions overlap, the behavior is undefined.

RETURNS
        <<memcpy>> returns a pointer to the first byte of the <[out]>
        region.

PORTABILITY
<<memcpy>> is ANSI C.

<<memcpy>> requires no supporting OS subroutines.

QUICKREF
        memcpy ansi pure
	*/

#include <_ansi.h>
#include <string.h>
#include <stdint.h>

/* Nonzero if either X or Y is not aligned on a "uintptr_t" boundary.  */
#define UNALIGNED(X, Y) \
  (((long)X & (sizeof (long) - 1)) | ((long)Y & (sizeof (long) - 1)))

/* How many bytes are copied each iteration of the 4X unrolled loop.  */
#define BIGBLOCKSIZE    (sizeof (long) << 2)

/* How many bytes are copied each iteration of the word copy loop.  */
#define LITTLEBLOCKSIZE (sizeof (long))

/* Threshhold for punting to the byte copier.  */
#define TOO_SMALL(LEN)  ((LEN) < BIGBLOCKSIZE)


_PTR
_DEFUN (memcpy, (dst0, src0, len0),
	_PTR __restrict dst0 _AND
	_CONST _PTR __restrict src0 _AND
	size_t len0)
{

#ifdef __CHERI__
  {
#define CAPABILITY_TYPE __intcap_t

    /* use byte copy if the block is smaller than a capability */
    if (len0 < sizeof(CAPABILITY_TYPE))
    {
      char *dst = (char *) dst0;
      _CONST char *src = (_CONST char *) src0;

      while (len0--)
        *dst++ = *src++;

      return dst0;
    }

    unsigned src_align = (long)src0 & (sizeof(CAPABILITY_TYPE) - 1);
    unsigned dst_align = (long)dst0 & (sizeof(CAPABILITY_TYPE) - 1);

    /* if the two pointers are aligned with respect to each other, then use
    * capabilities where possible
    */
    if (src_align == dst_align)
    {
      char *dst = (char *) dst0;
      _CONST char *src = (_CONST char *) src0;

        /* if the pointers are unaligned at the start, byte copy until aligned */
      if (src_align != 0)
      {
        unsigned count = sizeof(CAPABILITY_TYPE) - src_align;
        len0 -= count;
        while (count--)
          *dst++ = *src++;
      }

      _CONST CAPABILITY_TYPE* aligned_src = (_CONST CAPABILITY_TYPE*) src;
      CAPABILITY_TYPE* aligned_dst = (CAPABILITY_TYPE*) dst;

      /* src and dst are now aligned to a capability. if the size of the remaining
       * block is larger than 4 capabilities, unroll */
      while (len0 >= 4*sizeof(CAPABILITY_TYPE))
      {
        *aligned_dst++ = *aligned_src++;
        *aligned_dst++ = *aligned_src++;
        *aligned_dst++ = *aligned_src++;
        *aligned_dst++ = *aligned_src++;
        len0 -= 4*sizeof(CAPABILITY_TYPE);
      }

      /* copy any remaining capability sized pieces */
      while (len0 >= sizeof(CAPABILITY_TYPE))
      {
        *aligned_dst++ = *aligned_src++;
        len0 -= sizeof(CAPABILITY_TYPE);
      }
      /* copy any remaining bytes */
      dst = (char*)aligned_dst;
      src = (_CONST char*)aligned_src;

      while (len0--)
        *dst++ = *src++;

      return dst0;
    }
  }
#endif /* __CHERI__ */

#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
  char *dst = (char *) dst0;
  char *src = (char *) src0;

  _PTR save = dst0;

  while (len0--)
      *dst++ = *src++;

  return save;
#else

  // else fall back to the original capability unware implementation

  char *dst = dst0;
  _CONST char *src = src0;
  long *aligned_dst;
  _CONST long *aligned_src;

  /* If the size is small, or either SRC or DST is unaligned,
     then punt into the byte copy loop.  This should be rare.  */
  if (!TOO_SMALL(len0) && !UNALIGNED (src, dst))
    {
      aligned_dst = (long*)dst;
      aligned_src = (long*)src;

      /* Copy 4X long words at a time if possible.  */
      while (len0 >= BIGBLOCKSIZE)
        {
          *aligned_dst++ = *aligned_src++;
          *aligned_dst++ = *aligned_src++;
          *aligned_dst++ = *aligned_src++;
          *aligned_dst++ = *aligned_src++;
          len0 -= BIGBLOCKSIZE;
        }

      /* Copy one long word at a time if possible.  */
      while (len0 >= LITTLEBLOCKSIZE)
        {
          *aligned_dst++ = *aligned_src++;
          len0 -= LITTLEBLOCKSIZE;
        }

       /* Pick up any residual with a byte copier.  */
      dst = (char*)aligned_dst;
      src = (char*)aligned_src;
    }

  while (len0--)
    *dst++ = *src++;

  return dst0;
#endif /* not PREFER_SIZE_OVER_SPEED */
}
