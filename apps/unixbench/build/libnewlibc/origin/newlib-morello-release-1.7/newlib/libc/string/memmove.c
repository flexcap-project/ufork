/*
FUNCTION
	<<memmove>>---move possibly overlapping memory

INDEX
	memmove

ANSI_SYNOPSIS
	#include <string.h>
	void *memmove(void *<[dst]>, const void *<[src]>, size_t <[length]>);

TRAD_SYNOPSIS
	#include <string.h>
	void *memmove(<[dst]>, <[src]>, <[length]>)
	void *<[dst]>;
	void *<[src]>;
	size_t <[length]>;

DESCRIPTION
	This function moves <[length]> characters from the block of
	memory starting at <<*<[src]>>> to the memory starting at
	<<*<[dst]>>>. <<memmove>> reproduces the characters correctly
	at <<*<[dst]>>> even if the two areas overlap.


RETURNS
	The function returns <[dst]> as passed.

PORTABILITY
<<memmove>> is ANSI C.

<<memmove>> requires no supporting OS subroutines.

QUICKREF
	memmove ansi pure
*/

#include <string.h>
#include <_ansi.h>
#include <stddef.h>
#include <limits.h>
#include "local.h"

/* Nonzero if either X or Y is not aligned on a "long" boundary.  */
#define UNALIGNED(X, Y) \
  (((long)X & (sizeof (long) - 1)) | ((long)Y & (sizeof (long) - 1)))

/* How many bytes are copied each iteration of the 4X unrolled loop.  */
#define BIGBLOCKSIZE    (sizeof (long) << 2)

/* How many bytes are copied each iteration of the word copy loop.  */
#define LITTLEBLOCKSIZE (sizeof (long))

/* Threshhold for punting to the byte copier.  */
#define TOO_SMALL(LEN)  ((LEN) < BIGBLOCKSIZE)

/*SUPPRESS 20*/
_PTR
__inhibit_loop_to_libcall
_DEFUN (memmove, (dst_void, src_void, length),
	_PTR dst_void _AND
	_CONST _PTR src_void _AND
	size_t length)
{
#ifdef __CHERI__
#define CAPABILITY_TYPE __intcap_t
  {
    char *dst = dst_void;
    _CONST char *src = src_void;

    if (src < dst && dst < src + length)
    {
      // destructive overlap, copy backwards
      src += length;
      dst += length;

      /* use byte copy if the block is smaller than a capability */
      if (length < sizeof(CAPABILITY_TYPE))
      {
        while (length--)
          *--dst = *--src;

        return dst_void;
      }

      unsigned src_align = (long)src & (sizeof(CAPABILITY_TYPE) - 1);
      unsigned dst_align = (long)dst & (sizeof(CAPABILITY_TYPE) - 1);

      /* if the two pointers are aligned with respect to each other, then use
      * capabilities where possible
      */
      if (src_align == dst_align)
      {
        /* if the pointers are unaligned at the end, byte copy until aligned */
        if (src_align != 0)
        {
          unsigned count = src_align;
          length -= count;
          while (count--)
             *--dst = *--src;
        }

        _CONST CAPABILITY_TYPE* aligned_src = (_CONST CAPABILITY_TYPE*) src;
        CAPABILITY_TYPE* aligned_dst = (CAPABILITY_TYPE*) dst;

        /* src and dst are now aligned to a capability. if the size of the remaining
        * block is larger than 4 capabilities, unroll */
        while (length >= 4*sizeof(CAPABILITY_TYPE))
        {
          *--aligned_dst = *--aligned_src;
          *--aligned_dst = *--aligned_src;
          *--aligned_dst = *--aligned_src;
          *--aligned_dst = *--aligned_src;
          length -= 4*sizeof(CAPABILITY_TYPE);
        }

        /* copy any remaining capability sized pieces */
        while (length >= sizeof(CAPABILITY_TYPE))
        {
          *--aligned_dst = *--aligned_src;
          length -= sizeof(CAPABILITY_TYPE);
        }
        /* copy any remaining bytes */
        dst = (char*)aligned_dst;
        src = (_CONST char*)aligned_src;

        while (length--)
          *--dst = *--src;

        return dst_void;
      }
    }
    else // forward copy
    {
      /* use byte copy if the block is smaller than a capability */
      if (length < sizeof(CAPABILITY_TYPE))
      {
        while (length--)
          *dst++ = *src++;

        return dst_void;
      }

      unsigned src_align = (long)src & (sizeof(CAPABILITY_TYPE) - 1);
      unsigned dst_align = (long)dst & (sizeof(CAPABILITY_TYPE) - 1);

      /* if the two pointers are aligned with respect to each other, then use
       * capabilities where possible
       */
      if (src_align == dst_align)
      {
        /* if the pointers are unaligned at the start, byte copy until aligned */
        if (src_align != 0)
        {
          unsigned count = sizeof(CAPABILITY_TYPE) - src_align;
          length -= count;
          while (count--)
            *dst++ = *src++;
        }

        _CONST CAPABILITY_TYPE* aligned_src = (_CONST CAPABILITY_TYPE*) src;
        CAPABILITY_TYPE* aligned_dst = (CAPABILITY_TYPE*) dst;

        /* src and dst are now aligned to a capability. if the size of the remaining
        * block is larger than 4 capabilities, unroll */
        while (length >= 4*sizeof(CAPABILITY_TYPE))
        {
          *aligned_dst++ = *aligned_src++;
          *aligned_dst++ = *aligned_src++;
          *aligned_dst++ = *aligned_src++;
          *aligned_dst++ = *aligned_src++;
          length -= 4*sizeof(CAPABILITY_TYPE);
        }

        /* copy any remaining capability sized pieces */
        while (length >= sizeof(CAPABILITY_TYPE))
        {
          *aligned_dst++ = *aligned_src++;
          length -= sizeof(CAPABILITY_TYPE);
        }
        /* copy any remaining bytes */
        dst = (char*)aligned_dst;
        src = (_CONST char*)aligned_src;

        while (length--)
          *dst++ = *src++;

        return dst_void;
      }
    }
  }
#endif /* __CHERI__ */


#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)

  char *dst = dst_void;
  _CONST char *src = src_void;
  if (src < dst && dst < src + length)
    {
      /* Have to copy backwards */
      src += length;
      dst += length;
      while (length--)
	{
	  *--dst = *--src;
	}
    }
  else
    {
      while (length--)
	{
	  *dst++ = *src++;
	}
    }

  return dst_void;
#else
  char *dst = dst_void;
  _CONST char *src = src_void;
  long *aligned_dst;
  _CONST long *aligned_src;

  if (src < dst && dst < src + length)
    {
      /* Destructive overlap...have to copy backwards */
      src += length;
      dst += length;
      while (length--)
	{
	  *--dst = *--src;
	}
    }
  else
    {
      /* Use optimizing algorithm for a non-destructive copy to closely 
         match memcpy. If the size is small or either SRC or DST is unaligned,
         then punt into the byte copy loop.  This should be rare.  */
      if (!TOO_SMALL(length) && !UNALIGNED (src, dst))
        {
          aligned_dst = (long*)dst;
          aligned_src = (long*)src;

          /* Copy 4X long words at a time if possible.  */
          while (length >= BIGBLOCKSIZE)
            {
              *aligned_dst++ = *aligned_src++;
              *aligned_dst++ = *aligned_src++;
              *aligned_dst++ = *aligned_src++;
              *aligned_dst++ = *aligned_src++;
              length -= BIGBLOCKSIZE;
            }

          /* Copy one long word at a time if possible.  */
          while (length >= LITTLEBLOCKSIZE)
            {
              *aligned_dst++ = *aligned_src++;
              length -= LITTLEBLOCKSIZE;
            }

          /* Pick up any residual with a byte copier.  */
          dst = (char*)aligned_dst;
          src = (char*)aligned_src;
        }

      while (length--)
        {
          *dst++ = *src++;
        }
    }

  return dst_void;
#endif /* not PREFER_SIZE_OVER_SPEED */
}
