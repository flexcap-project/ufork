cmd_/root/.unikraft/apps/flexos-example/build/libredis_server/geo.o := ~/llvm1/llvm-project-releases/bin/clang -nostdlib -U __linux__ -U __FreeBSD__ -U __sun__ -fno-omit-frame-pointer -Werror=attributes -Wno-unused-variable -fpie -fno-stack-protector -Wall -Wextra -D __Unikraft__ -DUK_CODENAME="Tethys" -DUK_VERSION=0.5 -DUK_FULLVERSION=0.5.0~445952e9-custom -O2   -I/root/.unikraft/apps/flexos-example/build/include -I/root/.unikraft/unikraft/arch/arm/arm64/include -I/root/.unikraft/unikraft/include -I/root/.unikraft/unikraft/lib/flexos-core/include/ -I/root/.unikraft/unikraft/lib/sassy-fork/include/ -I/root/.unikraft/unikraft/lib/ukboot/include -I/root/.unikraft/unikraft/lib/ukswrand/include -I/root/.unikraft/unikraft/lib/posix-user/include -I/root/.unikraft/unikraft/lib/posix-user/musl-imported/include -I/root/.unikraft/unikraft/lib/posix-sysinfo/include -I/root/.unikraft/unikraft/lib/ukdebug/include -I/root/.unikraft/unikraft/lib/ukargparse/include -I/root/.unikraft/unikraft/lib/ukalloc/include -I/root/.unikraft/unikraft/lib/uksched/include -I/root/.unikraft/unikraft/lib/ukschedcoop/include -I/root/.unikraft/unikraft/lib/fdt/include -I/root/.unikraft/unikraft/lib/syscall_shim/include -I/root/.unikraft/unikraft/lib/vfscore/include -I/root/.unikraft/unikraft/lib/devfs/include -I/root/.unikraft/unikraft/lib/uklock/include -I/root/.unikraft/unikraft/lib/ukmpi/include -I/root/.unikraft/unikraft/lib/ukbus/include -I/root/.unikraft/unikraft/lib/uksglist/include -I/root/.unikraft/unikraft/lib/uknetdev/include -I/root/.unikraft/unikraft/lib/posix-libdl/include -I/root/.unikraft/unikraft/lib/uklibparam/include -I/root/.unikraft/unikraft/lib/uktime/include -I/root/.unikraft/unikraft/lib/uktime/musl-imported/include -I/root/.unikraft/unikraft/lib/ukblkdev/include -I/root/.unikraft/unikraft/lib/posix-process/include -I/root/.unikraft/unikraft/lib/posix-process/musl-imported/include -I/root/.unikraft/unikraft/lib/posix-process/musl-imported/arch/generic -I/root/.unikraft/unikraft/lib/uksp/include -I/root/.unikraft/unikraft/lib/uksignal/include -I/root/.unikraft/unikraft/lib/posix-mmap/include -I/root/.unikraft/libs/pthread-embedded/include -I/root/.unikraft/apps/flexos-example/build/libpthread-embedded/origin/pthread-embedded-44b41d760a433915d70a7be9809651b0a65e001d -I/root/.unikraft/apps/flexos-example/build/libpthread-embedded/origin/pthread-embedded-44b41d760a433915d70a7be9809651b0a65e001d/platform/helper -I/root/.unikraft/libs/lwip/include -I/root/.unikraft/libs/lwip/musl-imported/include -I/root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include -I/root/.unikraft/libs/newlib/include -I/root/.unikraft/libs/newlib/musl-imported/include -I/root/.unikraft/libs/newlib/musl-imported/arch/generic -I/root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include -I/root/.unikraft/apps/flexos-example/build/libcompiler_rt/origin/compiler-rt-1.7.0/compiler-rt/lib -I/root/.unikraft/apps/flexos-example/build/libcompiler_rt/origin/compiler-rt-1.7.0/compiler-rt/lib/builtins -I/root/.unikraft/libs/lib-tinyalloc -I/root/.unikraft/libs/lib-tinyalloc/include -I/root/.unikraft/apps/flexos-example/build/libtinyalloc/origin/tinyalloc-96450f32d80fe7d23f6aa5426046143e57801bc4 -I/root/.unikraft/libs/redis/include  -I/root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/deps/hiredis -I/root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/deps/lua/src   -D__ARM_64__  -DCC_VERSION=13.0  -target aarch64-none-elf -g0 -march=morello+c64 -mabi=purecap --sysroot=/root/alistair/llvm1/llvm-project-releases/aarch64-none-elf+morello+c64+purecap/lib/ -fomit-frame-pointer -g -DMISSING_SYSCALL_NAMES -DMALLOC_PROVIDED -D_POSIX_REALTIME_SIGNALS -D_WANT_IO_C99_FORMATS -D__LINUX_ERRNO_EXTENSIONS__ -D_LDBL_EQ_DBL -D_HAVE_LONG_DOUBLE -Wno-char-subscripts -D__DYNAMIC_REENT__ -DCONFIG_UK_NETDEV_SCRATCH_SIZE=4096   -Wno-unused-parameter -Wno-unused-variable -Wno-unused-value -Wno-implicit-fallthrough -Wno-char-subscripts -Wno-misleading-indentation  -Wno-missing-field-initializers -DREDIS_STATIC=    -g3 -D__LIBNAME__=libredis_server -D__BASENAME__=geo.c -c /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/geo.c -o /root/.unikraft/apps/flexos-example/build/libredis_server/geo.o -Wp,-MD,/root/.unikraft/apps/flexos-example/build/libredis_server/.geo.o.d

source_/root/.unikraft/apps/flexos-example/build/libredis_server/geo.o := /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/geo.c

deps_/root/.unikraft/apps/flexos-example/build/libredis_server/geo.o := \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/geo.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/server.h \
    $(wildcard include/config/default/dynamic/hz.h) \
    $(wildcard include/config/default/hz.h) \
    $(wildcard include/config/min/hz.h) \
    $(wildcard include/config/max/hz.h) \
    $(wildcard include/config/default/server/port.h) \
    $(wildcard include/config/default/tcp/backlog.h) \
    $(wildcard include/config/default/client/timeout.h) \
    $(wildcard include/config/default/dbnum.h) \
    $(wildcard include/config/max/line.h) \
    $(wildcard include/config/default/slowlog/log/slower/than.h) \
    $(wildcard include/config/default/slowlog/max/len.h) \
    $(wildcard include/config/default/max/clients.h) \
    $(wildcard include/config/authpass/max/len.h) \
    $(wildcard include/config/default/slave/priority.h) \
    $(wildcard include/config/default/repl/timeout.h) \
    $(wildcard include/config/default/repl/ping/slave/period.h) \
    $(wildcard include/config/run/id/size.h) \
    $(wildcard include/config/default/repl/backlog/size.h) \
    $(wildcard include/config/default/repl/backlog/time/limit.h) \
    $(wildcard include/config/repl/backlog/min/size.h) \
    $(wildcard include/config/bgsave/retry/delay.h) \
    $(wildcard include/config/default/pid/file.h) \
    $(wildcard include/config/default/syslog/ident.h) \
    $(wildcard include/config/default/cluster/config/file.h) \
    $(wildcard include/config/default/cluster/announce/ip.h) \
    $(wildcard include/config/default/cluster/announce/port.h) \
    $(wildcard include/config/default/cluster/announce/bus/port.h) \
    $(wildcard include/config/default/daemonize.h) \
    $(wildcard include/config/default/unix/socket/perm.h) \
    $(wildcard include/config/default/tcp/keepalive.h) \
    $(wildcard include/config/default/protected/mode.h) \
    $(wildcard include/config/default/logfile.h) \
    $(wildcard include/config/default/syslog/enabled.h) \
    $(wildcard include/config/default/stop/writes/on/bgsave/error.h) \
    $(wildcard include/config/default/rdb/compression.h) \
    $(wildcard include/config/default/rdb/checksum.h) \
    $(wildcard include/config/default/rdb/filename.h) \
    $(wildcard include/config/default/repl/diskless/sync.h) \
    $(wildcard include/config/default/repl/diskless/sync/delay.h) \
    $(wildcard include/config/default/slave/serve/stale/data.h) \
    $(wildcard include/config/default/slave/read/only.h) \
    $(wildcard include/config/default/slave/ignore/maxmemory.h) \
    $(wildcard include/config/default/slave/announce/ip.h) \
    $(wildcard include/config/default/slave/announce/port.h) \
    $(wildcard include/config/default/repl/disable/tcp/nodelay.h) \
    $(wildcard include/config/default/maxmemory.h) \
    $(wildcard include/config/default/maxmemory/samples.h) \
    $(wildcard include/config/default/lfu/log/factor.h) \
    $(wildcard include/config/default/lfu/decay/time.h) \
    $(wildcard include/config/default/aof/filename.h) \
    $(wildcard include/config/default/aof/no/fsync/on/rewrite.h) \
    $(wildcard include/config/default/aof/load/truncated.h) \
    $(wildcard include/config/default/aof/use/rdb/preamble.h) \
    $(wildcard include/config/default/active/rehashing.h) \
    $(wildcard include/config/default/aof/rewrite/incremental/fsync.h) \
    $(wildcard include/config/default/rdb/save/incremental/fsync.h) \
    $(wildcard include/config/default/min/slaves/to/write.h) \
    $(wildcard include/config/default/min/slaves/max/lag.h) \
    $(wildcard include/config/bindaddr/max.h) \
    $(wildcard include/config/min/reserved/fds.h) \
    $(wildcard include/config/default/latency/monitor/threshold.h) \
    $(wildcard include/config/default/slave/lazy/flush.h) \
    $(wildcard include/config/default/lazyfree/lazy/eviction.h) \
    $(wildcard include/config/default/lazyfree/lazy/expire.h) \
    $(wildcard include/config/default/lazyfree/lazy/server/del.h) \
    $(wildcard include/config/default/always/show/logo.h) \
    $(wildcard include/config/default/active/defrag.h) \
    $(wildcard include/config/default/defrag/threshold/lower.h) \
    $(wildcard include/config/default/defrag/threshold/upper.h) \
    $(wildcard include/config/default/defrag/ignore/bytes.h) \
    $(wildcard include/config/default/defrag/cycle/min.h) \
    $(wildcard include/config/default/defrag/cycle/max.h) \
    $(wildcard include/config/default/defrag/max/scan/fields.h) \
    $(wildcard include/config/default/proto/max/bulk/len.h) \
    $(wildcard include/config/fdset/incr.h) \
    $(wildcard include/config/repl/syncio/timeout.h) \
    $(wildcard include/config/default/verbosity.h) \
    $(wildcard include/config/default/aof/fsync.h) \
    $(wildcard include/config/default/hll/sparse/max/bytes.h) \
    $(wildcard include/config/default/maxmemory/policy.h) \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/fmacros.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/config.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/types.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/_ansi.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/newlib.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/config.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/ieeefp.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/features.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/_newlib_version.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/cdefs.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/_default_types.h \
  /root/.unikraft/libs/newlib/include/stddef.h \
  /root/.unikraft/libs/newlib/include/__stddef_max_align_t.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/_types.h \
  /root/.unikraft/libs/newlib/include/sys/_types.h \
  /root/.unikraft/libs/newlib/include/uk/_types.h \
  /root/.unikraft/unikraft/lib/uktime/include/uk/time_types.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/stdint.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_intsup.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_stdint.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_types.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/lock.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/types.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/select.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_sigset.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_timeval.h \
  /root/.unikraft/libs/newlib/include/sys/timespec.h \
  /root/.unikraft/libs/newlib/include/uk/_timespec.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/solarisfixes.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/rio.h \
  /root/.unikraft/libs/newlib/include/stdio.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/stdio.h \
  /root/.unikraft/libs/newlib/include/stdarg.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/reent.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/stdio.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/sds.h \
  /root/.unikraft/libs/newlib/include/stdlib.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/stdlib.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/stdlib.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/alloca.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/string.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/string.h \
  /root/.unikraft/unikraft/lib/uktime/musl-imported/include/time.h \
    $(wildcard include/config/libnolibc.h) \
  /root/.unikraft/unikraft/include/uk/config.h \
  /root/.unikraft/libs/newlib/include/time.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/locale.h \
  /root/.unikraft/libs/newlib/include/limits.h \
  /root/.unikraft/unikraft/include/uk/arch/limits.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/limits.h \
    $(wildcard include/config/stack/size/page/order.h) \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/intsizes.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/unistd.h \
  /root/.unikraft/libs/newlib/include/sys/unistd.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/unistd.h \
  /root/.unikraft/unikraft/lib/posix-user/include/uk/user.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/errno.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/errno.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/inttypes.h \
  /root/.unikraft/libs/pthread-embedded/include/pthread.h \
  /root/.unikraft/apps/flexos-example/build/libpthread-embedded/origin/pthread-embedded-44b41d760a433915d70a7be9809651b0a65e001d/pthread.h \
  /root/.unikraft/libs/pthread-embedded/include/pte_types.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/timeb.h \
  /root/.unikraft/libs/pthread-embedded/include/sched.h \
  /root/.unikraft/apps/flexos-example/build/libpthread-embedded/origin/pthread-embedded-44b41d760a433915d70a7be9809651b0a65e001d/sched.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/setjmp.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/setjmp.h \
  /root/.unikraft/libs/newlib/musl-imported/include/syslog.h \
  /root/.unikraft/libs/newlib/musl-imported/arch/generic/bits/alltypes.h \
  /root/.unikraft/libs/lwip/include/netinet/in.h \
    $(wildcard include/config/lwip/socket.h) \
    $(wildcard include/config/lwip/ipv6.h) \
  /root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include/lwip/sockets.h \
    $(wildcard include/config/have/libc.h) \
  /root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include/lwip/opt.h \
  /root/.unikraft/libs/lwip/include/lwipopts.h \
    $(wildcard include/config/lwip/heap.h) \
    $(wildcard include/config/lwip/pools.h) \
    $(wildcard include/config/lwip/heap/only.h) \
    $(wildcard include/config/lwip/pools/only.h) \
    $(wildcard include/config/lwip/nothreads.h) \
    $(wildcard include/config/lwip/netif/ext/status/callback.h) \
    $(wildcard include/config/lwip/have/loopif.h) \
    $(wildcard include/config/lwip/ipv4.h) \
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
  /root/.unikraft/unikraft/lib/uktime/musl-imported/include/sys/time.h \
  /root/.unikraft/libs/newlib/include/sys/time.h \
  /root/.unikraft/unikraft/lib/flexos-core/include/flexos/literals.h \
  /root/.unikraft/libs/newlib/include/sys/ioctl.h \
  /root/.unikraft/libs/newlib/musl-imported/include/sys/ioctl.h \
  /root/.unikraft/libs/newlib/musl-imported/arch/generic/bits/ioctl.h \
    $(wildcard include/config/liblwip.h) \
  /root/.unikraft/libs/newlib/include/sys/uio.h \
  /root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include/lwip/debug.h \
  /root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include/lwip/arch.h \
  /root/.unikraft/libs/lwip/include/arch/cc.h \
    $(wildcard include/config/libukdebug/enable/assert.h) \
  /root/.unikraft/unikraft/include/uk/arch/types.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/types.h \
  /root/.unikraft/unikraft/include/uk/plat/lcpu.h \
    $(wildcard include/config/have/smp.h) \
  /root/.unikraft/unikraft/include/uk/arch/time.h \
  /root/.unikraft/unikraft/include/uk/arch/lcpu.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/lcpu.h \
  /root/.unikraft/unikraft/include/uk/essentials.h \
    $(wildcard include/config/libnewlibc.h) \
    $(wildcard include/config/have/sched.h) \
  /root/.unikraft/libs/newlib/include/sys/param.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/param.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/syslimits.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/endian.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/param.h \
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
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/ctype.h \
  /root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include/lwip/ip_addr.h \
  /root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include/lwip/def.h \
  /root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include/lwip/ip4_addr.h \
  /root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include/lwip/ip6_addr.h \
  /root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include/lwip/ip6_zone.h \
  /root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include/lwip/netif.h \
  /root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include/lwip/err.h \
  /root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include/lwip/pbuf.h \
  /root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include/lwip/stats.h \
  /root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include/lwip/mem.h \
  /root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include/lwip/memp.h \
  /root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include/lwip/priv/memp_std.h \
  /root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include/lwip/priv/memp_priv.h \
  /root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include/lwip/priv/mem_priv.h \
  /root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include/lwip/inet.h \
  /root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include/lwip/errno.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/deps/lua/src/lua.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/deps/lua/src/luaconf.h \
  /root/.unikraft/libs/newlib/musl-imported/include/signal.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/ae.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/dict.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/adlist.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/zmalloc.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/anet.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/ziplist.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/intset.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/version.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/util.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/latency.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/sparkline.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/quicklist.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/rax.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/zipmap.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/sha1.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/endianconv.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/crc64.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/stream.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/listpack.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/rdb.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/geohash_helper.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/geohash.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/src/debugmacro.h \

/root/.unikraft/apps/flexos-example/build/libredis_server/geo.o: $(deps_/root/.unikraft/apps/flexos-example/build/libredis_server/geo.o)

$(deps_/root/.unikraft/apps/flexos-example/build/libredis_server/geo.o):
