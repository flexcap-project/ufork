#include <compat/posix/arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
int inet_pton(int af, const char *src, void *dst);
int getaddrinfo_kernel(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
void freeaddrinfo_kernel(struct addrinfo *res);

#ifdef __cplusplus
}
#endif
