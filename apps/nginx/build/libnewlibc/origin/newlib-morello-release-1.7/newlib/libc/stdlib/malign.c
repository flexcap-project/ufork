#ifndef MALLOC_PROVIDED
/* malign.c -- a wrapper for memalign_r.  */

#include <_ansi.h>
#include <reent.h>
#include <stdlib.h>
#include <malloc.h>

#ifndef _REENT_ONLY

_PTR
_DEFUN (memalign, (align, nbytes),
	size_t align _AND
	size_t nbytes)
{
  return _memalign_r (_REENT, align, nbytes);
}

int
_DEFUN (posix_memalign, (memptr, align, nbytes),
        _PTR* memptr _AND
	size_t align _AND
	size_t nbytes)
{
  if (nbytes==0) {
    *memptr = (void *) 0;
    return 0;
  }

  if (align < sizeof(void *))
    align = sizeof(void *);

  void *ptr =  _memalign_r (_REENT, align, nbytes);

  *memptr = ptr;

  return ptr == (void *)0;
}

#endif
#endif
