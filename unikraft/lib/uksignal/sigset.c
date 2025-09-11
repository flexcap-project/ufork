/* taken from newlib */

#include <errno.h>
#include <uk/uk_signal.h>
#include <signal.h>

int sigemptyset_kernel(sigset_t *set)
{
#ifdef CONFIG_LIBSASSYFORK_TOCTTOU
	struct sigset_t* set_old = set;
	if (set)
		set = sfork_copy_from_user(set, sizeof(unsigned long));
#endif
	uk_sigemptyset(set);
#ifdef CONFIG_LIBSASSYFORK_TOCTTOU
	if (set)
		set = sfork_copy_to_user(set, set_old, sizeof(unsigned long));
#endif
	return 0;
}

int sigemptyset(sigset_t *set)__attribute__((section(".app_libs_text")))
{
	if (sfork_syscalls.sigemptyset)
		return sfork_syscalls.sigemptyset(set);
	else
		return sigemptyset_kernel(set);
}

int sigfillset(sigset_t *set)
{
	uk_sigfillset(set);
	return 0;
}

int sigaddset_kernel(sigset_t *set, int signo)
{
#ifdef CONFIG_LIBSASSYFORK_TOCTTOU
	struct sigset_t* set_old = set;
	if (set)
		set = sfork_copy_from_user(set, sizeof(unsigned long));
#endif
	if (signo >= NSIG || signo <= 0) {
		errno = EINVAL;
#ifdef CONFIG_LIBSASSYFORK_TOCTTOU
	if (set)
		set = sfork_copy_to_user(set, set_old, sizeof(unsigned long));
#endif
		return -1;
	}

	uk_sigaddset(set, signo);
#ifdef CONFIG_LIBSASSYFORK_TOCTTOU
	if (set)
		set = sfork_copy_to_user(set, set_old, sizeof(unsigned long));
#endif
	return 0;
}

int sigaddset(sigset_t *set, int signo)__attribute__((section(".app_libs_text")))
{
	if (sfork_syscalls.sigaddset)
		return sfork_syscalls.sigaddset(set, signo);
	else
		return sigaddset_kernel(set, signo);
}

int sigdelset(sigset_t *set, int signo)
{
	if (signo >= NSIG || signo <= 0) {
		errno = EINVAL;
		return -1;
	}

	uk_sigdelset(set, signo);
	return 0;
}

int sigismember(const sigset_t *set, int signo)
{
	if (signo >= NSIG || signo <= 0) {
		errno = EINVAL;
		return -1;
	}

	if (uk_sigismember(set, signo))
		return 1;
	else
		return 0;
}
