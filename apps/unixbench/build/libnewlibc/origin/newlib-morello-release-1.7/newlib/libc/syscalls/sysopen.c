/* connector for open */

#include <reent.h>
#include <fcntl.h>

#ifdef _HAVE_STDC

/* The prototype in <fcntl.h> uses ..., so we must correspond.  */

#include <stdarg.h>

int
_DEFUN (open, (file, flags, ...),
        const char *file _AND
        int flags _DOTS)
{
  va_list ap;
  int ret;
  int mode = 0;

  va_start (ap, flags);
  if (flags & O_CREAT)
    mode = va_arg(ap, int);

  ret = _open_r (_REENT, file, flags, mode);
  va_end (ap);
  return ret;
}

#else /* ! _HAVE_STDC */

int 
open (file, flags, mode)
     const char *file;
     int flags;
     int mode;
{
  return _open_r (_REENT, file, flags, mode);
}

#endif /* ! _HAVE_STDC */
