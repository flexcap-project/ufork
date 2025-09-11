/* Auto generated file. DO NOT EDIT */

#include <uk/syscall.h>
#include <uk/print.h>
#include <stdlib.h>

long (*uk_syscall_r_fn(long nr))(void)
{
	switch (nr) {
	case SYS_getegid:
		return (long (*)(void)) uk_syscall_r_getegid;
	case SYS_geteuid:
		return (long (*)(void)) uk_syscall_r_geteuid;
	case SYS_getgid:
		return (long (*)(void)) uk_syscall_r_getgid;
	case SYS_getuid:
		return (long (*)(void)) uk_syscall_r_getuid;
	case SYS_uname:
		return (long (*)(void)) uk_syscall_r_uname;
	case SYS_sethostname:
		return (long (*)(void)) uk_syscall_r_sethostname;
	case SYS_nanosleep:
		return (long (*)(void)) uk_syscall_r_nanosleep;
	case SYS_clock_gettime:
		return (long (*)(void)) uk_syscall_r_clock_gettime;
	case SYS_getpgid:
		return (long (*)(void)) uk_syscall_r_getpgid;
	case SYS_setpgid:
		return (long (*)(void)) uk_syscall_r_setpgid;
	case SYS_setsid:
		return (long (*)(void)) uk_syscall_r_setsid;
	case SYS_getsid:
		return (long (*)(void)) uk_syscall_r_getsid;
	case SYS_setpriority:
		return (long (*)(void)) uk_syscall_r_setpriority;
	case SYS_getpgrp:
		return (long (*)(void)) uk_syscall_r_getpgrp;
	case SYS_getpriority:
		return (long (*)(void)) uk_syscall_r_getpriority;
	case SYS_getpid:
		return (long (*)(void)) uk_syscall_r_getpid;
	case SYS_getppid:
		return (long (*)(void)) uk_syscall_r_getppid;
	default:
		return NULL;
	}
}
