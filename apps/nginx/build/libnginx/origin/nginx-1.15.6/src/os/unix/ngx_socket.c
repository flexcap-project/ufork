
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <flexos/isolation.h>
#include <ngx_config.h>
#include <ngx_core.h>


/*
 * ioctl(FIONBIO) sets a non-blocking mode with the single syscall
 * while fcntl(F_SETFL, O_NONBLOCK) needs to learn the current state
 * using fcntl(F_GETFL).
 *
 * ioctl() and fcntl() are syscalls at least in FreeBSD 2.x, Linux 2.2
 * and Solaris 7.
 *
 * ioctl() in Linux 2.4 and 2.6 uses BKL, however, fcntl(F_SETFL) uses it too.
 */

#include <flexos/isolation.h>

static inline int setsockopt_wrapper(int sockfd, int level, int optname,
             const void *optval, socklen_t optlen)
{
	int ret;
	ret= setsockopt(sockfd, level, optname, optval, optlen);
	return ret;
}

#define setsockopt setsockopt_wrapper


#if (NGX_HAVE_FIONBIO)

int
ngx_nonblocking(ngx_socket_t s)
{
    int  nb;

    nb = 1;

    int ret = ioctl(s, FIONBIO, &nb);

    return ret;
}


int
ngx_blocking(ngx_socket_t s)
{
    int  nb;

    nb = 0;
    int ret = ioctl(s, FIONBIO, &nb);

    return ret;
}

#endif


#if (NGX_FREEBSD)

int
ngx_tcp_nopush(ngx_socket_t s)
{
    int  tcp_nopush;

    tcp_nopush = 1;

    int ret = setsockopt(s, IPPROTO_TCP, TCP_NOPUSH,
                      (const void *) &tcp_nopush, sizeof(int));

    return ret;
}


int
ngx_tcp_push(ngx_socket_t s)
{
    int  tcp_nopush;

    tcp_nopush = 0;

    int ret = setsockopt(s, IPPROTO_TCP, TCP_NOPUSH,
                      (const void *) &tcp_nopush, sizeof(int));

    return ret;
}

#elif (NGX_LINUX)


int
ngx_tcp_nopush(ngx_socket_t s)
{
    int  cork;

    cork = 1;

    int ret = setsockopt(s, IPPROTO_TCP, TCP_CORK,
                      (const void *) &cork, sizeof(int));

    return ret;
}


int
ngx_tcp_push(ngx_socket_t s)
{
    int  cork;

    cork = 0;

    int ret = setsockopt(s, IPPROTO_TCP, TCP_CORK,
                      (const void *) &cork, sizeof(int));

    return ret;
}

#else

int
ngx_tcp_nopush(ngx_socket_t s)
{
    return 0;
}


int
ngx_tcp_push(ngx_socket_t s)
{
    return 0;
}

#endif
