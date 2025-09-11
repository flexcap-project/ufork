/* Auto generated file. DO NOT EDIT */

#include <stddef.h>

#include <uk/syscall.h>

const char *uk_syscall_name_p(long nr)
{
	switch (nr) {
	case SYS_getegid:
		return "getegid";
	case SYS_geteuid:
		return "geteuid";
	case SYS_getgid:
		return "getgid";
	case SYS_getuid:
		return "getuid";
	case SYS_uname:
		return "uname";
	case SYS_sethostname:
		return "sethostname";
	case SYS_nanosleep:
		return "nanosleep";
	case SYS_clock_gettime:
		return "clock_gettime";
	case SYS_getpgid:
		return "getpgid";
	case SYS_setpgid:
		return "setpgid";
	case SYS_setsid:
		return "setsid";
	case SYS_getsid:
		return "getsid";
	case SYS_setpriority:
		return "setpriority";
	case SYS_getpgrp:
		return "getpgrp";
	case SYS_getpriority:
		return "getpriority";
	case SYS_getpid:
		return "getpid";
	case SYS_getppid:
		return "getppid";
	default:
		return NULL;
	}
}
