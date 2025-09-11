cmd_/root/.unikraft/apps/nginx/build/libnginx/ngx_http_memcached_module.o := ~/llvm1/llvm-project-releases/bin/clang -nostdlib -U __linux__ -U __FreeBSD__ -U __sun__ -fno-omit-frame-pointer -Werror=attributes -Wno-unused-variable -fpie -fno-stack-protector -Wall -Wextra -D __Unikraft__ -DUK_CODENAME="Tethys" -DUK_VERSION=0.5 -DUK_FULLVERSION=0.5.0~811c131c-custom -O2   -I/root/.unikraft/apps/nginx/build/include -I/root/.unikraft/unikraft/arch/arm/arm64/include -I/root/.unikraft/unikraft/include -I/root/.unikraft/unikraft/lib/flexos-core/include/ -I/root/.unikraft/unikraft/lib/sassy-fork/include/ -I/root/.unikraft/unikraft/lib/ukboot/include -I/root/.unikraft/unikraft/lib/ukswrand/include -I/root/.unikraft/unikraft/lib/posix-user/include -I/root/.unikraft/unikraft/lib/posix-user/musl-imported/include -I/root/.unikraft/unikraft/lib/posix-sysinfo/include -I/root/.unikraft/unikraft/lib/ukdebug/include -I/root/.unikraft/unikraft/lib/ukargparse/include -I/root/.unikraft/unikraft/lib/ukalloc/include -I/root/.unikraft/unikraft/lib/uksched/include -I/root/.unikraft/unikraft/lib/ukschedcoop/include -I/root/.unikraft/unikraft/lib/fdt/include -I/root/.unikraft/unikraft/lib/syscall_shim/include -I/root/.unikraft/unikraft/lib/vfscore/include -I/root/.unikraft/unikraft/lib/devfs/include -I/root/.unikraft/unikraft/lib/uklock/include -I/root/.unikraft/unikraft/lib/ukmpi/include -I/root/.unikraft/unikraft/lib/ukbus/include -I/root/.unikraft/unikraft/lib/uksglist/include -I/root/.unikraft/unikraft/lib/uknetdev/include -I/root/.unikraft/unikraft/lib/posix-libdl/include -I/root/.unikraft/unikraft/lib/uklibparam/include -I/root/.unikraft/unikraft/lib/uktime/include -I/root/.unikraft/unikraft/lib/uktime/musl-imported/include -I/root/.unikraft/unikraft/lib/ukblkdev/include -I/root/.unikraft/unikraft/lib/posix-process/include -I/root/.unikraft/unikraft/lib/posix-process/musl-imported/include -I/root/.unikraft/unikraft/lib/posix-process/musl-imported/arch/generic -I/root/.unikraft/unikraft/lib/uksp/include -I/root/.unikraft/unikraft/lib/uksignal/include -I/root/.unikraft/unikraft/lib/posix-mmap/include -I/root/.unikraft/libs/pthread-embedded/include -I/root/.unikraft/apps/nginx/build/libpthread-embedded/origin/pthread-embedded-44b41d760a433915d70a7be9809651b0a65e001d -I/root/.unikraft/apps/nginx/build/libpthread-embedded/origin/pthread-embedded-44b41d760a433915d70a7be9809651b0a65e001d/platform/helper -I/root/.unikraft/libs/lwip/include -I/root/.unikraft/libs/lwip/musl-imported/include -I/root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include -I/root/.unikraft/libs/newlib/include -I/root/.unikraft/libs/newlib/musl-imported/include -I/root/.unikraft/libs/newlib/musl-imported/arch/generic -I/root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include -I/root/.unikraft/libs/nginx/include -I/root/.unikraft/apps/nginx/build/libcompiler_rt/origin/compiler-rt-1.7.0/compiler-rt/lib -I/root/.unikraft/apps/nginx/build/libcompiler_rt/origin/compiler-rt-1.7.0/compiler-rt/lib/builtins -I/root/.unikraft/libs/lib-tinyalloc -I/root/.unikraft/libs/lib-tinyalloc/include -I/root/.unikraft/apps/nginx/build/libtinyalloc/origin/tinyalloc-96450f32d80fe7d23f6aa5426046143e57801bc4  -I/root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core -I/root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/event -I/root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/event/modules -I/root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/http -I/root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/http/modules -I/root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/http/modules/perl -I/root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/http/v2 -I/root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/os/unix   -D__ARM_64__  -DCC_VERSION=13.0  -target aarch64-none-elf -g0 -march=morello+c64 -mabi=purecap --sysroot=/root/alistair/llvm1/llvm-project-releases/aarch64-none-elf+morello+c64+purecap/lib/ -fomit-frame-pointer -g -DMISSING_SYSCALL_NAMES -DMALLOC_PROVIDED -D_POSIX_REALTIME_SIGNALS -D_WANT_IO_C99_FORMATS -D__LINUX_ERRNO_EXTENSIONS__ -D_LDBL_EQ_DBL -D_HAVE_LONG_DOUBLE -Wno-char-subscripts -D__DYNAMIC_REENT__ -DCONFIG_UK_NETDEV_SCRATCH_SIZE=4096     -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-value    -g3 -D__LIBNAME__=libnginx -D__BASENAME__=ngx_http_memcached_module.c -c /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/http/modules/ngx_http_memcached_module.c -o /root/.unikraft/apps/nginx/build/libnginx/ngx_http_memcached_module.o -Wp,-MD,/root/.unikraft/apps/nginx/build/libnginx/.ngx_http_memcached_module.o.d

source_/root/.unikraft/apps/nginx/build/libnginx/ngx_http_memcached_module.o := /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/http/modules/ngx_http_memcached_module.c

deps_/root/.unikraft/apps/nginx/build/libnginx/ngx_http_memcached_module.o := \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_config.h \
  /root/.unikraft/libs/nginx/include/ngx_auto_headers.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/os/unix/ngx_linux_config.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/types.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/_ansi.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/newlib.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/config.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/ieeefp.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/features.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/_newlib_version.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/cdefs.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/_default_types.h \
  /root/.unikraft/libs/newlib/include/stddef.h \
  /root/.unikraft/libs/newlib/include/__stddef_max_align_t.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/_types.h \
  /root/.unikraft/libs/newlib/include/sys/_types.h \
  /root/.unikraft/libs/newlib/include/uk/_types.h \
  /root/.unikraft/unikraft/lib/uktime/include/uk/time_types.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/stdint.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_intsup.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_stdint.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_types.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/lock.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/types.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/select.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_sigset.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_timeval.h \
  /root/.unikraft/libs/newlib/include/sys/timespec.h \
  /root/.unikraft/libs/newlib/include/uk/_timespec.h \
  /root/.unikraft/unikraft/lib/uktime/musl-imported/include/sys/time.h \
    $(wildcard include/config/libnolibc.h) \
  /root/.unikraft/unikraft/include/uk/config.h \
  /root/.unikraft/libs/newlib/include/sys/time.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/reent.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/unistd.h \
  /root/.unikraft/libs/newlib/include/sys/unistd.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/unistd.h \
  /root/.unikraft/unikraft/lib/posix-user/include/uk/user.h \
  /root/.unikraft/libs/newlib/include/stdarg.h \
  /root/.unikraft/libs/newlib/include/stdio.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/stdio.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/stdio.h \
  /root/.unikraft/libs/newlib/include/stdlib.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/stdlib.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/stdlib.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/alloca.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/ctype.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/errno.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/errno.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/string.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/string.h \
  /root/.unikraft/libs/newlib/musl-imported/include/signal.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/pwd.h \
  /root/.unikraft/unikraft/lib/posix-user/musl-imported/include/grp.h \
  /root/.unikraft/libs/newlib/include/dirent.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/glob.h \
  /root/.unikraft/unikraft/lib/vfscore/include/sys/vfs.h \
  /root/.unikraft/libs/newlib/include/sys/statfs.h \
  /root/.unikraft/libs/newlib/include/sys/statvfs.h \
  /root/.unikraft/libs/newlib/include/endian.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/endian.h \
  /root/.unikraft/libs/newlib/include/sys/uio.h \
  /root/.unikraft/libs/newlib/include/sys/stat.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/stat.h \
  /root/.unikraft/unikraft/lib/uktime/musl-imported/include/time.h \
  /root/.unikraft/libs/newlib/include/time.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/locale.h \
  /root/.unikraft/libs/newlib/include/sys/_timespec.h \
  /root/.unikraft/libs/newlib/include/fcntl.h \
    $(wildcard include/config/arch/x86/64.h) \
    $(wildcard include/config/arch/arm/64.h) \
    $(wildcard include/config/arch/arm/32.h) \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/fcntl.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/fcntl.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_default_fcntl.h \
  /root/.unikraft/libs/newlib/include/sys/wait.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/wait.h \
  /root/.unikraft/unikraft/lib/posix-process/musl-imported/include/sys/resource.h \
  /root/.unikraft/unikraft/lib/posix-process/musl-imported/arch/generic/bits/resource.h \
  /root/.unikraft/unikraft/lib/posix-mmap/include/sys/mman.h \
  /root/.unikraft/libs/pthread-embedded/include/sched.h \
  /root/.unikraft/apps/nginx/build/libpthread-embedded/origin/pthread-embedded-44b41d760a433915d70a7be9809651b0a65e001d/sched.h \
  /root/.unikraft/libs/pthread-embedded/include/pte_types.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/timeb.h \
  /root/.unikraft/libs/lwip/include/sys/socket.h \
    $(wildcard include/config/lwip/socket.h) \
    $(wildcard include/config/have/libc.h) \
  /root/.unikraft/libs/newlib/include/poll.h \
  /root/.unikraft/libs/newlib/include/sys/poll.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/lwip/sockets.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/lwip/opt.h \
  /root/.unikraft/libs/lwip/include/lwipopts.h \
    $(wildcard include/config/lwip/heap.h) \
    $(wildcard include/config/lwip/pools.h) \
    $(wildcard include/config/lwip/heap/only.h) \
    $(wildcard include/config/lwip/pools/only.h) \
    $(wildcard include/config/lwip/nothreads.h) \
    $(wildcard include/config/lwip/netif/ext/status/callback.h) \
    $(wildcard include/config/lwip/have/loopif.h) \
    $(wildcard include/config/lwip/ipv4.h) \
    $(wildcard include/config/lwip/ipv6.h) \
    $(wildcard include/config/lwip/udp.h) \
    $(wildcard include/config/lwip/tcp.h) \
    $(wildcard include/config/lwip/tcp/mss.h) \
    $(wildcard include/config/lwip/wnd/scale.h) \
    $(wildcard include/config/lwip/wnd/scale/factor.h) \
    $(wildcard include/config/lwip/num/tcpcon.h) \
    $(wildcard include/config/lwip/num/tcplisteners.h) \
    $(wildcard include/config/lwip/dns.h) \
    $(wildcard include/config/lwip/dns/max/servers.h) \
    $(wildcard include/config/lwip/dns/table/size.h) \
    $(wildcard include/config/lwip/icmp.h) \
    $(wildcard include/config/lwip/igmp.h) \
    $(wildcard include/config/lwip/snmp.h) \
    $(wildcard include/config/lwip/dhcp.h) \
    $(wildcard include/config/lwip/debug.h) \
    $(wildcard include/config/lwip/mainloop/debug.h) \
    $(wildcard include/config/lwip/if/debug.h) \
    $(wildcard include/config/lwip/ip/debug.h) \
    $(wildcard include/config/lwip/udp/debug.h) \
    $(wildcard include/config/lwip/tcp/debug.h) \
    $(wildcard include/config/lwip/sys/debug.h) \
    $(wildcard include/config/lwip/api/debug.h) \
    $(wildcard include/config/lwip/service/debug.h) \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/inttypes.h \
  /root/.unikraft/unikraft/lib/flexos-core/include/flexos/literals.h \
  /root/.unikraft/libs/newlib/include/sys/ioctl.h \
  /root/.unikraft/libs/newlib/musl-imported/include/sys/ioctl.h \
  /root/.unikraft/libs/newlib/musl-imported/arch/generic/bits/ioctl.h \
    $(wildcard include/config/liblwip.h) \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/lwip/debug.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/lwip/arch.h \
  /root/.unikraft/libs/lwip/include/arch/cc.h \
    $(wildcard include/config/libukdebug/enable/assert.h) \
  /root/.unikraft/unikraft/include/uk/arch/types.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/intsizes.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/types.h \
  /root/.unikraft/unikraft/include/uk/plat/lcpu.h \
    $(wildcard include/config/have/smp.h) \
  /root/.unikraft/unikraft/include/uk/arch/time.h \
  /root/.unikraft/unikraft/include/uk/arch/limits.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/limits.h \
    $(wildcard include/config/stack/size/page/order.h) \
  /root/.unikraft/unikraft/include/uk/arch/lcpu.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/lcpu.h \
  /root/.unikraft/unikraft/include/uk/essentials.h \
    $(wildcard include/config/libnewlibc.h) \
    $(wildcard include/config/have/sched.h) \
  /root/.unikraft/libs/newlib/include/sys/param.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/param.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/syslimits.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/param.h \
  /root/.unikraft/unikraft/lib/ukdebug/include/uk/print.h \
    $(wildcard include/config/libukdebug/printd.h) \
    $(wildcard include/config/libukdebug/printk/crit.h) \
    $(wildcard include/config/libukdebug/printk/err.h) \
    $(wildcard include/config/libukdebug/printk/warn.h) \
    $(wildcard include/config/libukdebug/printk/info.h) \
    $(wildcard include/config/libukdebug/printk.h) \
  /root/.unikraft/unikraft/lib/ukdebug/include/uk/assert.h \
  /root/.unikraft/unikraft/include/uk/plat/bootstrap.h \
  /root/.unikraft/unikraft/lib/ukswrand/include/uk/swrand.h \
  /root/.unikraft/unikraft/include/uk/plat/time.h \
    $(wildcard include/config/hz.h) \
  /root/.unikraft/unikraft/lib/flexos-core/include/flexos/isolation.h \
    $(wildcard include/config/libflexos/intelpku.h) \
    $(wildcard include/config/libflexos/morello.h) \
    $(wildcard include/config/libflexos/vmept.h) \
  /root/.unikraft/unikraft/lib/flexos-core/include/flexos/impl/morello.h \
  /root/.unikraft/unikraft/lib/flexos-core/include/flexos/impl/morello-impl.h \
  /root/.unikraft/unikraft/lib/ukalloc/include/uk/alloc.h \
    $(wildcard include/config/libukalloc/ifstats.h) \
    $(wildcard include/config/libukalloc/ifmalloc.h) \
  /root/.unikraft/libs/newlib/include/limits.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/lwip/ip_addr.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/lwip/def.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/lwip/ip4_addr.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/lwip/ip6_addr.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/lwip/netif.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/lwip/err.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/lwip/pbuf.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/lwip/stats.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/lwip/mem.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/lwip/memp.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/lwip/priv/memp_std.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/lwip/priv/memp_priv.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/lwip/priv/mem_priv.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/lwip/inet.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/lwip/errno.h \
  /root/.unikraft/libs/lwip/include/netinet/in.h \
  /root/.unikraft/libs/lwip/include/netinet/tcp.h \
  /root/.unikraft/libs/lwip/include/arpa/inet.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/compat/posix/arpa/inet.h \
  /root/.unikraft/libs/lwip/include/netdb.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/compat/posix/netdb.h \
  /root/.unikraft/apps/nginx/build/liblwip/origin/lwip-2.1.2/src/include/lwip/netdb.h \
  /root/.unikraft/libs/newlib/musl-imported/include/sys/un.h \
  /root/.unikraft/libs/newlib/musl-imported/arch/generic/bits/alltypes.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/malloc.h \
  /root/.unikraft/apps/nginx/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/malloc.h \
  /root/.unikraft/libs/newlib/musl-imported/include/crypt.h \
  /root/.unikraft/unikraft/lib/posix-sysinfo/include/sys/utsname.h \
  /root/.unikraft/unikraft/lib/posix-libdl/include/dlfcn.h \
  /root/.unikraft/libs/nginx/include/ngx_auto_config.h \
    $(wildcard include/config/libnginx/debug.h) \
    $(wildcard include/config/libnginx/http/upstream/random.h) \
    $(wildcard include/config/libnginx/http/v2.h) \
    $(wildcard include/config/libssl.h) \
    $(wildcard include/config/libnginx/http/ssl.h) \
    $(wildcard include/config/libnginx/http/gzip.h) \
    $(wildcard include/config/libnginx/http/ssi.h) \
    $(wildcard include/config/libcrypto.h) \
    $(wildcard include/config/libpcre.h) \
    $(wildcard include/config/libzlib.h) \
  /root/.unikraft/apps/nginx/build/libpthread-embedded/origin/pthread-embedded-44b41d760a433915d70a7be9809651b0a65e001d/semaphore.h \
  /root/.unikraft/unikraft/lib/posix-process/include/sys/prctl.h \
  /root/.unikraft/libs/lwip/musl-imported/include/sys/sendfile.h \
  /root/.unikraft/libs/newlib/musl-imported/include/sys/eventfd.h \
  /root/.unikraft/libs/newlib/include/sys/syscall.h \
    $(wildcard include/config/libsyscall/shim.h) \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_core.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/os/unix/ngx_errno.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/os/unix/ngx_atomic.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/os/unix/ngx_thread.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_rbtree.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/os/unix/ngx_time.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/os/unix/ngx_socket.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_string.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/os/unix/ngx_files.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/os/unix/ngx_shmem.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/os/unix/ngx_process.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/os/unix/ngx_setaffinity.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/os/unix/ngx_setproctitle.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/os/unix/ngx_user.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/os/unix/ngx_dlopen.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_parse.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_parse_time.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_log.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/os/unix/ngx_alloc.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_palloc.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_buf.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_queue.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_array.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_list.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_hash.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_file.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_crc.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_crc32.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_murmurhash.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_radix_tree.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_times.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_rwlock.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_shmtx.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_slab.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_inet.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_cycle.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_resolver.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/os/unix/ngx_process_cycle.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_conf_file.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_module.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/nginx.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_open_file_cache.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/os/unix/ngx_os.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/os/unix/ngx_linux.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_connection.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_syslog.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/core/ngx_proxy_protocol.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/http/ngx_http.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/http/ngx_http_variables.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/http/ngx_http_config.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/http/ngx_http_request.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/http/ngx_http_script.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/http/ngx_http_upstream.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/event/ngx_event.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/event/ngx_event_timer.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/event/ngx_event_posted.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/event/ngx_event_connect.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/event/ngx_event_pipe.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/http/ngx_http_upstream_round_robin.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/http/ngx_http_core_module.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/http/v2/ngx_http_v2.h \
  /root/.unikraft/apps/nginx/build/libnginx/origin/nginx-1.15.6/src/http/modules/ngx_http_ssi_filter_module.h \

/root/.unikraft/apps/nginx/build/libnginx/ngx_http_memcached_module.o: $(deps_/root/.unikraft/apps/nginx/build/libnginx/ngx_http_memcached_module.o)

$(deps_/root/.unikraft/apps/nginx/build/libnginx/ngx_http_memcached_module.o):
