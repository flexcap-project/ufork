cmd_/root/.unikraft/apps/micropython/build/lib9pfs/9pfs_vfsops.o := ~/llvm2/llvm-project-releases-morello-baremetal-release-1.8/bin/clang -nostdlib -U __linux__ -U __FreeBSD__ -U __sun__ -fno-omit-frame-pointer -Werror=attributes -Wno-unused-variable -fpie -fno-stack-protector -Wall -Wextra -D __Unikraft__ -DUK_CODENAME="Tethys" -DUK_VERSION=0.5 -DUK_FULLVERSION=0.5.0~1cc073e22-custom -O2   -I/root/.unikraft/apps/micropython/build/include -I/root/.unikraft/unikraft/arch/arm/arm64/include -I/root/.unikraft/unikraft/include -I/root/.unikraft/unikraft/lib/flexos-core/include/ -I/root/.unikraft/unikraft/lib/sassy-fork/include/ -I/root/.unikraft/unikraft/lib/ukboot/include -I/root/.unikraft/unikraft/lib/ukboot/plat -I/root/.unikraft/unikraft/lib/ukswrand/include -I/root/.unikraft/unikraft/lib/posix-user/include -I/root/.unikraft/unikraft/lib/posix-user/musl-imported/include -I/root/.unikraft/unikraft/lib/posix-sysinfo/include -I/root/.unikraft/unikraft/lib/ukdebug/include -I/root/.unikraft/unikraft/lib/ukargparse/include -I/root/.unikraft/unikraft/lib/ukalloc/include -I/root/.unikraft/unikraft/lib/uksched/include -I/root/.unikraft/unikraft/lib/ukschedcoop/include -I/root/.unikraft/unikraft/lib/fdt/include -I/root/.unikraft/unikraft/lib/syscall_shim/include -I/root/.unikraft/unikraft/lib/vfscore/include -I/root/.unikraft/unikraft/lib/devfs/include -I/root/.unikraft/unikraft/lib/uklock/include -I/root/.unikraft/unikraft/lib/ukmpi/include -I/root/.unikraft/unikraft/lib/ukbus/include -I/root/.unikraft/unikraft/lib/uksglist/include -I/root/.unikraft/unikraft/lib/uknetdev/include -I/root/.unikraft/unikraft/lib/uk9p/include -I/root/.unikraft/unikraft/lib/posix-libdl/include -I/root/.unikraft/unikraft/lib/uklibparam/include -I/root/.unikraft/unikraft/lib/uktime/include -I/root/.unikraft/unikraft/lib/uktime/musl-imported/include -I/root/.unikraft/unikraft/lib/ukblkdev/include -I/root/.unikraft/unikraft/lib/posix-process/include -I/root/.unikraft/unikraft/lib/posix-process/musl-imported/include -I/root/.unikraft/unikraft/lib/posix-process/musl-imported/arch/generic -I/root/.unikraft/unikraft/lib/uksp/include -I/root/.unikraft/unikraft/lib/uksignal/include -I/root/.unikraft/unikraft/lib/posix-mmap/include -I/root/.unikraft/libs/pthread-embedded/include -I/root/.unikraft/apps/micropython/build/libpthread-embedded/origin/pthread-embedded-44b41d760a433915d70a7be9809651b0a65e001d -I/root/.unikraft/apps/micropython/build/libpthread-embedded/origin/pthread-embedded-44b41d760a433915d70a7be9809651b0a65e001d/platform/helper -I/root/.unikraft/libs/lwip/include -I/root/.unikraft/libs/lwip/musl-imported/include -I/root/.unikraft/apps/micropython/build/liblwip/origin/lwip-2.1.2/src/include -I/root/.unikraft/libs/newlib/include -I/root/.unikraft/libs/newlib/musl-imported/include -I/root/.unikraft/libs/newlib/musl-imported/arch/generic -I/root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include -I/root/.unikraft/apps/micropython/build/libcompiler_rt/origin/compiler-rt-1.7.0/compiler-rt/lib -I/root/.unikraft/apps/micropython/build/libcompiler_rt/origin/compiler-rt-1.7.0/compiler-rt/lib/builtins -I/root/.unikraft/libs/lib-tinyalloc -I/root/.unikraft/libs/lib-tinyalloc/include -I/root/.unikraft/apps/micropython/build/libtinyalloc/origin/tinyalloc-96450f32d80fe7d23f6aa5426046143e57801bc4     -D__ARM_64__  -DCC_VERSION=14.0  -target aarch64-none-elf -g0 -march=morello+c64 -mabi=purecap --sysroot=/root/alistair/llvm2/llvm-project-releases-morello-baremetal-release-1.8/aarch64-none-elf+morello+c64+purecap/lib/ -fomit-frame-pointer -g -DMISSING_SYSCALL_NAMES -DMALLOC_PROVIDED -D_POSIX_REALTIME_SIGNALS -D_LDBL_EQ_DBL -D_HAVE_LONG_DOUBLE -Wno-char-subscripts -D__DYNAMIC_REENT__ -DCONFIG_UK_NETDEV_SCRATCH_SIZE=4096       -g3 -D__LIBNAME__=lib9pfs -D__BASENAME__=9pfs_vfsops.c -c /root/.unikraft/unikraft/lib/9pfs/9pfs_vfsops.c -o /root/.unikraft/apps/micropython/build/lib9pfs/9pfs_vfsops.o -Wp,-MD,/root/.unikraft/apps/micropython/build/lib9pfs/.9pfs_vfsops.o.d

source_/root/.unikraft/apps/micropython/build/lib9pfs/9pfs_vfsops.o := /root/.unikraft/unikraft/lib/9pfs/9pfs_vfsops.c

deps_/root/.unikraft/apps/micropython/build/lib9pfs/9pfs_vfsops.o := \
  /root/.unikraft/unikraft/include/uk/config.h \
  /root/.unikraft/unikraft/include/uk/errptr.h \
  /root/.unikraft/unikraft/include/uk/arch/types.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/intsizes.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/types.h \
  /root/.unikraft/unikraft/lib/uk9p/include/uk/9p.h \
  /root/.unikraft/libs/newlib/include/stdarg.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/stdint.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/_default_types.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/features.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/_newlib_version.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_intsup.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_stdint.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/string.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/_ansi.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/newlib.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/config.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/ieeefp.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/reent.h \
  /root/.unikraft/libs/newlib/include/stddef.h \
  /root/.unikraft/libs/newlib/include/__stddef_max_align_t.h \
  /root/.unikraft/libs/newlib/include/sys/_types.h \
  /root/.unikraft/libs/newlib/include/uk/_types.h \
  /root/.unikraft/unikraft/lib/uktime/include/uk/time_types.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_types.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/_types.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/lock.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/cdefs.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/string.h \
  /root/.unikraft/unikraft/lib/uk9p/include/uk/9p_core.h \
  /root/.unikraft/libs/newlib/include/limits.h \
  /root/.unikraft/unikraft/include/uk/arch/limits.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/limits.h \
    $(wildcard include/config/stack/size/page/order.h) \
  /root/.unikraft/unikraft/lib/ukdebug/include/uk/assert.h \
    $(wildcard include/config/libukdebug/enable/assert.h) \
  /root/.unikraft/unikraft/include/uk/plat/bootstrap.h \
  /root/.unikraft/unikraft/include/uk/essentials.h \
    $(wildcard include/config/libnewlibc.h) \
    $(wildcard include/config/have/sched.h) \
  /root/.unikraft/libs/newlib/include/sys/param.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/param.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/syslimits.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/endian.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/param.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/inttypes.h \
  /root/.unikraft/unikraft/include/uk/arch/lcpu.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/lcpu.h \
  /root/.unikraft/unikraft/lib/ukdebug/include/uk/print.h \
    $(wildcard include/config/libukdebug/printd.h) \
    $(wildcard include/config/libukdebug/printk/crit.h) \
    $(wildcard include/config/libukdebug/printk/err.h) \
    $(wildcard include/config/libukdebug/printk/warn.h) \
    $(wildcard include/config/libukdebug/printk/info.h) \
    $(wildcard include/config/libukdebug/printk.h) \
  /root/.unikraft/unikraft/lib/flexos-core/include/flexos/literals.h \
  /root/.unikraft/unikraft/lib/uk9p/include/uk/9pdev.h \
  /root/.unikraft/libs/newlib/include/stdbool.h \
  /root/.unikraft/unikraft/lib/ukalloc/include/uk/alloc.h \
    $(wildcard include/config/libukalloc/ifstats.h) \
    $(wildcard include/config/libukalloc/ifmalloc.h) \
    $(wildcard include/config/libflexos/intelpku.h) \
    $(wildcard include/config/libflexos/morello.h) \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/types.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/types.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/select.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_sigset.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_timeval.h \
  /root/.unikraft/libs/newlib/include/sys/timespec.h \
  /root/.unikraft/libs/newlib/include/uk/_timespec.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/errno.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/errno.h \
  /root/.unikraft/unikraft/lib/sassy-fork/include/sfork.h \
  /root/.unikraft/unikraft/lib/posix-process/musl-imported/include/sys/resource.h \
  /root/.unikraft/unikraft/lib/uktime/musl-imported/include/sys/time.h \
    $(wildcard include/config/libnolibc.h) \
  /root/.unikraft/libs/newlib/include/sys/time.h \
  /root/.unikraft/unikraft/lib/posix-process/musl-imported/arch/generic/bits/resource.h \
  /root/.unikraft/unikraft/include/uk/arch/spinlock.h \
    $(wildcard include/config/have/smp.h) \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/spinlock.h \
  /root/.unikraft/unikraft/include/uk/arch/atomic.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/atomic.h \
  /root/.unikraft/unikraft/lib/flexos-core/include/flexos/impl/morello.h \
  /root/.unikraft/unikraft/lib/uk9p/include/uk/9pdev_core.h \
    $(wildcard include/config/libuksched.h) \
  /root/.unikraft/unikraft/include/uk/bitmap.h \
  /root/.unikraft/unikraft/include/uk/bitops.h \
  /root/.unikraft/unikraft/include/uk/bitcount.h \
  /root/.unikraft/unikraft/include/uk/list.h \
  /root/.unikraft/unikraft/include/uk/compat_list.h \
  /root/.unikraft/unikraft/lib/uksched/include/uk/wait_types.h \
  /root/.unikraft/unikraft/include/uk/plat/irq.h \
  /root/.unikraft/unikraft/lib/uk9p/include/uk/9preq.h \
  /root/.unikraft/unikraft/include/uk/refcount.h \
    $(wildcard include/config/libukdebug.h) \
  /root/.unikraft/unikraft/lib/uk9p/include/uk/9pfid.h \
  /root/.unikraft/unikraft/lib/uk9p/include/uk/9pdev_trans.h \
  /root/.unikraft/unikraft/lib/vfscore/include/vfscore/mount.h \
  /root/.unikraft/libs/newlib/include/sys/mount.h \
  /root/.unikraft/libs/newlib/include/sys/statfs.h \
  /root/.unikraft/libs/newlib/include/sys/statvfs.h \
  /root/.unikraft/libs/newlib/include/endian.h \
  /root/.unikraft/unikraft/lib/vfscore/include/vfscore/vnode.h \
    $(wildcard include/config/libposix/event.h) \
  /root/.unikraft/libs/newlib/include/sys/stat.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/stat.h \
  /root/.unikraft/unikraft/lib/uktime/musl-imported/include/time.h \
  /root/.unikraft/libs/newlib/include/time.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/locale.h \
  /root/.unikraft/libs/newlib/include/sys/_timespec.h \
  /root/.unikraft/libs/newlib/include/dirent.h \
  /root/.unikraft/unikraft/lib/uklock/include/uk/mutex.h \
    $(wildcard include/config/libuklock/mutex.h) \
  /root/.unikraft/unikraft/lib/flexos-core/include/flexos/isolation.h \
    $(wildcard include/config/libflexos/vmept.h) \
  /root/.unikraft/unikraft/lib/flexos-core/include/flexos/impl/morello-impl.h \
  /root/.unikraft/unikraft/include/uk/plat/lcpu.h \
    $(wildcard include/config/ukplat/lcpu/maxcount.h) \
  /root/.unikraft/unikraft/include/uk/arch/time.h \
  /root/.unikraft/unikraft/lib/uksched/include/uk/thread.h \
    $(wildcard include/config/libuksignal.h) \
  /root/.unikraft/unikraft/include/uk/plat/thread.h \
  /root/.unikraft/libs/newlib/include/stdlib.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/stdlib.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/stdlib.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/alloca.h \
  /root/.unikraft/unikraft/lib/uksignal/include/uk/uk_signal.h \
  /root/.unikraft/unikraft/lib/uksignal/include/uk/bits/sigset.h \
  /root/.unikraft/libs/newlib/musl-imported/include/signal.h \
  /root/.unikraft/unikraft/lib/uksched/include/uk/thread_attr.h \
  /root/.unikraft/unikraft/include/uk/page.h \
  /root/.unikraft/unikraft/lib/uksched/include/uk/wait.h \
  /root/.unikraft/unikraft/lib/uksched/include/uk/sched.h \
  /root/.unikraft/unikraft/include/uk/plat/time.h \
    $(wildcard include/config/hz.h) \
  /root/.unikraft/unikraft/lib/vfscore/include/vfscore/uio.h \
  /root/.unikraft/libs/newlib/include/sys/uio.h \
  /root/.unikraft/unikraft/lib/vfscore/include/vfscore/dentry.h \
  /root/.unikraft/unikraft/lib/9pfs/9pfs.h \
  /root/.unikraft/unikraft/lib/vfscore/include/vfscore/prex.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/unistd.h \
  /root/.unikraft/libs/newlib/include/sys/unistd.h \
  /root/.unikraft/apps/micropython/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/unistd.h \
  /root/.unikraft/unikraft/lib/posix-user/include/uk/user.h \

/root/.unikraft/apps/micropython/build/lib9pfs/9pfs_vfsops.o: $(deps_/root/.unikraft/apps/micropython/build/lib9pfs/9pfs_vfsops.o)

$(deps_/root/.unikraft/apps/micropython/build/lib9pfs/9pfs_vfsops.o):
