/* Auto generated file. DO NOT EDIT */


#include <uk/syscall.h>
#include <uk/print.h>

long uk_syscall6_r(long nr, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6)
{
	(void) arg1;
	(void) arg2;
	(void) arg3;
	(void) arg4;
	(void) arg5;
	(void) arg6;

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
		return uk_syscall_r_uname(arg1);
	case SYS_sethostname:
		return uk_syscall_r_sethostname(arg1, arg2);
	case SYS_nanosleep:
		return uk_syscall_r_nanosleep(arg1, arg2);
	case SYS_clock_gettime:
		return uk_syscall_r_clock_gettime(arg1, arg2);
	case SYS_getpgid:
		return uk_syscall_r_getpgid(arg1);
	case SYS_setpgid:
		return uk_syscall_r_setpgid(arg1, arg2);
	case SYS_setsid:
		return uk_syscall_r_setsid();
	case SYS_getsid:
		return uk_syscall_r_getsid(arg1);
	case SYS_setpriority:
		return uk_syscall_r_setpriority(arg1, arg2, arg3);
	case SYS_getpgrp:
		return uk_syscall_r_getpgrp();
	case SYS_getpriority:
		return uk_syscall_r_getpriority(arg1, arg2);
	case SYS_getpid:
		return uk_syscall_r_getpid();
	case SYS_getppid:
		return uk_syscall_r_getppid();
	default:
		uk_pr_debug("syscall \"%s\" is not available\n", uk_syscall_name(nr));
		return -ENOSYS;
	}
}

#if CONFIG_LIBSYSCALL_SHIM_HANDLER
#include <uk/plat/syscall.h>
#include <uk/assert.h>
#include <arch/regmap_linuxabi.h>

void ukplat_syscall_handler(struct __regs *r)
{
	UK_ASSERT(r);

	uk_pr_debug("Binary system call request \"%s\" (%lu) at ip:%p (arg0=0x%lx, arg1=0x%lx, ...)\n",
		    uk_syscall_name(r->rsyscall), r->rsyscall,
		    (void *) r->rip, r->rarg0, r->rarg1);
	r->rret0 = uk_syscall6_r(r->rsyscall,
				 r->rarg0, r->rarg1, r->rarg2,
				 r->rarg3, r->rarg4, r->rarg5);
}
#endif /* CONFIG_LIBSYSCALL_SHIM_HANDLER */
