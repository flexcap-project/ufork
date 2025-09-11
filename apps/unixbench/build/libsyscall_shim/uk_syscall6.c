/* Auto generated file. DO NOT EDIT */


#include <uk/syscall.h>
#include <uk/print.h>

long uk_syscall6(long nr, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6)
{
	(void) arg1;
	(void) arg2;
	(void) arg3;
	(void) arg4;
	(void) arg5;
	(void) arg6;

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
		return uk_syscall_e_uname(arg1);
	case SYS_sethostname:
		return uk_syscall_e_sethostname(arg1, arg2);
	case SYS_nanosleep:
		return uk_syscall_e_nanosleep(arg1, arg2);
	case SYS_clock_gettime:
		return uk_syscall_e_clock_gettime(arg1, arg2);
	case SYS_getpgid:
		return uk_syscall_e_getpgid(arg1);
	case SYS_setpgid:
		return uk_syscall_e_setpgid(arg1, arg2);
	case SYS_setsid:
		return uk_syscall_e_setsid();
	case SYS_getsid:
		return uk_syscall_e_getsid(arg1);
	case SYS_setpriority:
		return uk_syscall_e_setpriority(arg1, arg2, arg3);
	case SYS_getpgrp:
		return uk_syscall_e_getpgrp();
	case SYS_getpriority:
		return uk_syscall_e_getpriority(arg1, arg2);
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
