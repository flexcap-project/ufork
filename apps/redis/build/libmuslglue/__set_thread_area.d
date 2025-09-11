cmd_/root/.unikraft/apps/flexos-example/build/libmuslglue/__set_thread_area.o := ~/llvm-project-releases/bin1/clang -nostdlib -U __linux__ -U __FreeBSD__ -U __sun__ -fno-omit-frame-pointer -Werror=attributes -Wno-unused-variable -fno-stack-protector -Wall -Wextra -D __Unikraft__ -DUK_CODENAME="Tethys" -DUK_VERSION=0.5 -DUK_FULLVERSION=0.5.0~2dfa1ed-custom -O2 -fno-builtin -fno-PIC  -I/root/.unikraft/apps/flexos-example/build/libmusl/origin/musl-libc-morello-release-1.7.0//src/include -I/root/.unikraft/apps/flexos-example/build/libmusl/origin/musl-libc-morello-release-1.7.0//src/internal -I/root/.unikraft/apps/flexos-example/build/include -I/root/.unikraft/unikraft/arch/arm/arm64/include -I/root/.unikraft/unikraft/include -I/root/.unikraft/unikraft/lib/flexos-core/include/ -I/root/.unikraft/unikraft/lib/ukboot/include -I/root/.unikraft/unikraft/lib/ukswrand/include -I/root/.unikraft/unikraft/lib/posix-user/include -I/root/.unikraft/unikraft/lib/posix-user/musl-imported/include -I/root/.unikraft/unikraft/lib/posix-sysinfo/include -I/root/.unikraft/unikraft/lib/ukdebug/include -I/root/.unikraft/unikraft/lib/ukargparse/include -I/root/.unikraft/unikraft/lib/ukalloc/include -I/root/.unikraft/unikraft/lib/ukallocregion/include -I/root/.unikraft/unikraft/lib/uksched/include -I/root/.unikraft/unikraft/lib/ukschedcoop/include -I/root/.unikraft/apps/flexos-example/build/libsyscall_shim/include -I/root/.unikraft/unikraft/lib/syscall_shim/include -I/root/.unikraft/unikraft/lib/vfscore/include -I/root/.unikraft/unikraft/lib/devfs/include -I/root/.unikraft/unikraft/lib/uklock/include -I/root/.unikraft/unikraft/lib/uklibparam/include -I/root/.unikraft/unikraft/lib/uktime/include -I/root/.unikraft/unikraft/lib/uktime/musl-imported/include -I/root/.unikraft/unikraft/lib/posix-process/include -I/root/.unikraft/unikraft/lib/posix-process/musl-imported/include -I/root/.unikraft/unikraft/lib/posix-process/musl-imported/arch/generic -I/root/.unikraft/unikraft/lib/uksp/include -I/root/.unikraft/unikraft/lib/uksignal/include -I/root/.unikraft/apps/flexos-example/build/libsodium/origin/libsodium-1.0.18/src/libsodium/include -I/root/.unikraft/apps/flexos-example/build/libmusl/origin/musl-libc-morello-release-1.7.0//arch/aarch64 -I/root/.unikraft/apps/flexos-example/build/libmusl/origin/musl-libc-morello-release-1.7.0//arch/generic -I/root/.unikraft/libs/lib-musl/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/core/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/mq/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/dirent/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/misc/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/temp/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/multibyte/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/process/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/mman/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/stdlib/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/fcntl/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/stdio/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/conf/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/math/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/aio/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/stat/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/crypt/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/select/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/malloc/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/unistd/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/signal/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/thread/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/legacy/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/env/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/time/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/errno/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/ipc/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/prng/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/ldso/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/passwd/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/fenv/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/ctype/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/linux/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/internal/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/network/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/sched/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/string/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/termios/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/exit/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/setjmp/include -I/root/.unikraft/apps/flexos-example/build/libmusl/include/locale/include -I/root/.unikraft/apps/flexos-example/build/libcompiler_rt/origin/compiler-rt-1.7.0/compiler-rt/lib -I/root/.unikraft/apps/flexos-example/build/libcompiler_rt/origin/compiler-rt-1.7.0/compiler-rt/lib/builtins -I/root/.unikraft/apps/flexos-example/build/libmusl/origin/musl-libc-morello-release-1.7.0//src/internal    -D__ARM_64__  -DCC_VERSION=13.0  -target aarch64-none-elf -g0 -march=morello+c64 -mabi=purecap -ffreestanding -Wno-unused-parameter -Wno-parentheses -Wno-builtin-macro-redefined -DCONFIG_UK_NETDEV_SCRATCH_SIZE=0   -Wno-unused-value    -g3 -D__LIBNAME__=libmuslglue -D__BASENAME__=__set_thread_area.c -c /root/.unikraft/libs/lib-musl/__set_thread_area.c -o /root/.unikraft/apps/flexos-example/build/libmuslglue/__set_thread_area.o -Wp,-MD,/root/.unikraft/apps/flexos-example/build/libmuslglue/.__set_thread_area.o.d

source_/root/.unikraft/apps/flexos-example/build/libmuslglue/__set_thread_area.o := /root/.unikraft/libs/lib-musl/__set_thread_area.c

deps_/root/.unikraft/apps/flexos-example/build/libmuslglue/__set_thread_area.o := \
  /root/.unikraft/unikraft/lib/uksched/include/uk/sched.h \
    $(wildcard include/config/libflexos/vmept.h) \
  /root/.unikraft/unikraft/lib/ukalloc/include/uk/alloc.h \
    $(wildcard include/config/libukalloc/ifstats.h) \
    $(wildcard include/config/libukalloc/ifmalloc.h) \
    $(wildcard include/config/libflexos/intelpku.h) \
    $(wildcard include/config/libflexos/morello.h) \
  /root/.unikraft/apps/flexos-example/build/libmusl/include/dirent/include/stddef.h \
  /root/.unikraft/apps/flexos-example/build/libmusl/origin/musl-libc-morello-release-1.7.0//arch/aarch64/bits/alltypes.h \
  /root/.unikraft/apps/flexos-example/build/libmusl/include/dirent/include/stdint.h \
  /root/.unikraft/apps/flexos-example/build/libmusl/origin/musl-libc-morello-release-1.7.0//arch/aarch64/bits/stdint.h \
  /root/.unikraft/apps/flexos-example/build/libmusl/include/core/include/sys/types.h \
  /root/.unikraft/apps/flexos-example/build/libmusl/origin/musl-libc-morello-release-1.7.0//src/include/features.h \
  /root/.unikraft/apps/flexos-example/build/libmusl/origin/musl-libc-morello-release-1.7.0//src/include/../../include/features.h \
  /root/.unikraft/apps/flexos-example/build/libmusl/include/misc/include/endian.h \
  /root/.unikraft/apps/flexos-example/build/libmusl/include/select/include/sys/select.h \
  /root/.unikraft/apps/flexos-example/build/libmusl/origin/musl-libc-morello-release-1.7.0//src/include/errno.h \
  /root/.unikraft/apps/flexos-example/build/libmusl/origin/musl-libc-morello-release-1.7.0//src/include/../../include/errno.h \
  /root/.unikraft/apps/flexos-example/build/libmusl/origin/musl-libc-morello-release-1.7.0//arch/generic/bits/errno.h \
  /root/.unikraft/unikraft/include/uk/config.h \
  /root/.unikraft/unikraft/lib/ukdebug/include/uk/assert.h \
    $(wildcard include/config/libukdebug/enable/assert.h) \
  /root/.unikraft/unikraft/include/uk/plat/bootstrap.h \
  /root/.unikraft/unikraft/include/uk/arch/types.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/intsizes.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/types.h \
  /root/.unikraft/unikraft/include/uk/essentials.h \
    $(wildcard include/config/libnewlibc.h) \
    $(wildcard include/config/have/sched.h) \
  /root/.unikraft/unikraft/include/uk/arch/lcpu.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/lcpu.h \
  /root/.unikraft/unikraft/lib/ukdebug/include/uk/print.h \
    $(wildcard include/config/libukdebug/printd.h) \
    $(wildcard include/config/libukdebug/printk/crit.h) \
    $(wildcard include/config/libukdebug/printk/err.h) \
    $(wildcard include/config/libukdebug/printk/warn.h) \
    $(wildcard include/config/libukdebug/printk/info.h) \
    $(wildcard include/config/libukdebug/printk.h) \
  /root/.unikraft/apps/flexos-example/build/libmusl/include/mq/include/stdarg.h \
  /root/.unikraft/unikraft/lib/flexos-core/include/flexos/literals.h \
  /root/.unikraft/unikraft/lib/flexos-core/include/flexos/impl/morello.h \
  /root/.unikraft/unikraft/lib/uksched/include/uk/thread.h \
    $(wildcard include/config/libuksignal.h) \
  /root/.unikraft/unikraft/lib/flexos-core/include/flexos/isolation.h \
  /root/.unikraft/unikraft/lib/flexos-core/include/flexos/impl/morello-impl.h \
  /root/.unikraft/apps/flexos-example/build/libmusl/include/core/include/stdbool.h \
  /root/.unikraft/unikraft/include/uk/arch/time.h \
  /root/.unikraft/unikraft/include/uk/arch/limits.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/limits.h \
    $(wildcard include/config/stack/size/page/order.h) \
  /root/.unikraft/unikraft/include/uk/plat/thread.h \
  /root/.unikraft/apps/flexos-example/build/libmusl/origin/musl-libc-morello-release-1.7.0//src/include/stdlib.h \
  /root/.unikraft/apps/flexos-example/build/libmusl/origin/musl-libc-morello-release-1.7.0//src/include/../../include/stdlib.h \
  /root/.unikraft/apps/flexos-example/build/libmusl/include/core/include/alloca.h \
  /root/.unikraft/unikraft/lib/uksignal/include/uk/uk_signal.h \
  /root/.unikraft/unikraft/include/uk/list.h \
  /root/.unikraft/unikraft/include/uk/arch/atomic.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/atomic.h \
  /root/.unikraft/unikraft/include/uk/compat_list.h \
  /root/.unikraft/unikraft/lib/uksignal/include/uk/bits/sigset.h \
  /root/.unikraft/apps/flexos-example/build/libmusl/origin/musl-libc-morello-release-1.7.0//src/include/signal.h \
  /root/.unikraft/apps/flexos-example/build/libmusl/origin/musl-libc-morello-release-1.7.0//src/include/../../include/signal.h \
  /root/.unikraft/apps/flexos-example/build/libmusl/origin/musl-libc-morello-release-1.7.0//arch/aarch64/bits/signal.h \
  /root/.unikraft/unikraft/lib/uksched/include/uk/thread_attr.h \
  /root/.unikraft/unikraft/lib/uksched/include/uk/wait_types.h \
  /root/.unikraft/unikraft/include/uk/page.h \
  /root/.unikraft/unikraft/include/uk/plat/time.h \
    $(wildcard include/config/hz.h) \

/root/.unikraft/apps/flexos-example/build/libmuslglue/__set_thread_area.o: $(deps_/root/.unikraft/apps/flexos-example/build/libmuslglue/__set_thread_area.o)

$(deps_/root/.unikraft/apps/flexos-example/build/libmuslglue/__set_thread_area.o):
