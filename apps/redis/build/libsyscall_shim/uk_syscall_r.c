/* Auto generated file. DO NOT EDIT */


#include <uk/syscall.h>
#include <uk/print.h>

long uk_vsyscall_r(long nr, va_list arg)
{
	(void) arg;

	switch (nr) {
	case SYS_getegid:
		return uk_syscall_r_getegid();
	case SYS_geteuid:
		return uk_syscall_r_geteuid();
	case SYS_getgid:
		return uk_syscall_r_getgid();
	case SYS_getuid:
		return uk_syscall_r_getuid();
	case SYS_uname:
		return uk_syscall_r_uname(va_arg(arg, long));
	case SYS_sethostname:
		return uk_syscall_r_sethostname(va_arg(arg, long), va_arg(arg, long));
	case SYS_nanosleep:
		return uk_syscall_r_nanosleep(va_arg(arg, long), va_arg(arg, long));
	case SYS_clock_gettime:
		return uk_syscall_r_clock_gettime(va_arg(arg, long), va_arg(arg, long));
	case SYS_getpgid:
		return uk_syscall_r_getpgid(va_arg(arg, long));
	case SYS_setpgid:
		return uk_syscall_r_setpgid(va_arg(arg, long), va_arg(arg, long));
	case SYS_setsid:
		return uk_syscall_r_setsid();
	case SYS_getsid:
		return uk_syscall_r_getsid(va_arg(arg, long));
	case SYS_setpriority:
		return uk_syscall_r_setpriority(va_arg(arg, long), va_arg(arg, long), va_arg(arg, long));
	case SYS_getpgrp:
		return uk_syscall_r_getpgrp();
	case SYS_getpriority:
		return uk_syscall_r_getpriority(va_arg(arg, long), va_arg(arg, long));
	case SYS_getpid:
		return uk_syscall_r_getpid();
	case SYS_getppid:
		return uk_syscall_r_getppid();
	default:
		uk_pr_debug("syscall \"%s\" is not available\n", uk_syscall_name(nr));
		return -ENOSYS;
	}
}

long uk_syscall_r(long nr, ...)
{
	long ret;
	va_list ap;

	va_start(ap, nr);
	ret = uk_vsyscall_r(nr, ap);
	va_end(ap);
	return ret;
}
