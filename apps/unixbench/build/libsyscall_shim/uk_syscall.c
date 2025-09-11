/* Auto generated file. DO NOT EDIT */


#include <uk/syscall.h>
#include <uk/print.h>

long uk_vsyscall(long nr, va_list arg)
{
	(void) arg;

	__maybe_unused long a1, a2, a3, a4, a5, a6;
	switch (nr) {
	case SYS_getegid:
		return uk_syscall_e_getegid();
	case SYS_geteuid:
		return uk_syscall_e_geteuid();
	case SYS_getgid:
		return uk_syscall_e_getgid();
	case SYS_getuid:
		return uk_syscall_e_getuid();
	case SYS_uname:
		a1 = va_arg(arg, long);
		return uk_syscall_e_uname(a1);
	case SYS_sethostname:
		a1 = va_arg(arg, long);
		a2 = va_arg(arg, long);
		return uk_syscall_e_sethostname(a1, a2);
	case SYS_nanosleep:
		a1 = va_arg(arg, long);
		a2 = va_arg(arg, long);
		return uk_syscall_e_nanosleep(a1, a2);
	case SYS_clock_gettime:
		a1 = va_arg(arg, long);
		a2 = va_arg(arg, long);
		return uk_syscall_e_clock_gettime(a1, a2);
	case SYS_getpgid:
		a1 = va_arg(arg, long);
		return uk_syscall_e_getpgid(a1);
	case SYS_setpgid:
		a1 = va_arg(arg, long);
		a2 = va_arg(arg, long);
		return uk_syscall_e_setpgid(a1, a2);
	case SYS_setsid:
		return uk_syscall_e_setsid();
	case SYS_getsid:
		a1 = va_arg(arg, long);
		return uk_syscall_e_getsid(a1);
	case SYS_setpriority:
		a1 = va_arg(arg, long);
		a2 = va_arg(arg, long);
		a3 = va_arg(arg, long);
		return uk_syscall_e_setpriority(a1, a2, a3);
	case SYS_getpgrp:
		return uk_syscall_e_getpgrp();
	case SYS_getpriority:
		a1 = va_arg(arg, long);
		a2 = va_arg(arg, long);
		return uk_syscall_e_getpriority(a1, a2);
	case SYS_getpid:
		return uk_syscall_e_getpid();
	case SYS_getppid:
		return uk_syscall_e_getppid();
	default:
		uk_pr_debug("syscall \"%s\" is not available\n", uk_syscall_name(nr));
		errno = -ENOSYS;
		return -1;
	}
}

long uk_syscall(long nr, ...)
{
	long ret;
	va_list ap;

	va_start(ap, nr);
	ret = uk_vsyscall(nr, ap);
	va_end(ap);
	return ret;
}
