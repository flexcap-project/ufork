cmd_/root/.unikraft/apps/flexos-example/build/libcompiler_rt/divxc3.o := ~/llvm1/llvm-project-releases/bin/clang -nostdlib -U __linux__ -U __FreeBSD__ -U __sun__ -fno-omit-frame-pointer -Werror=attributes -Wno-unused-variable -fno-stack-protector -Wall -Wextra -D __Unikraft__ -DUK_CODENAME="Tethys" -DUK_VERSION=0.5 -DUK_FULLVERSION=0.5.0~c49b548 -O2 -fno-builtin -fno-PIC   -I/root/.unikraft/apps/flexos-example/build/include -I/root/.unikraft/unikraft/arch/arm/arm64/include -I/root/.unikraft/unikraft/include -I/root/.unikraft/unikraft/lib/flexos-core/include/ -I/root/.unikraft/unikraft/lib/ukboot/include -I/root/.unikraft/unikraft/lib/ukswrand/include -I/root/.unikraft/unikraft/lib/posix-user/include -I/root/.unikraft/unikraft/lib/posix-user/musl-imported/include -I/root/.unikraft/unikraft/lib/posix-sysinfo/include -I/root/.unikraft/unikraft/lib/ukdebug/include -I/root/.unikraft/unikraft/lib/ukargparse/include -I/root/.unikraft/unikraft/lib/ukalloc/include -I/root/.unikraft/unikraft/lib/ukallocregion/include -I/root/.unikraft/unikraft/lib/uksched/include -I/root/.unikraft/unikraft/lib/ukschedcoop/include -I/root/.unikraft/unikraft/lib/syscall_shim/include -I/root/.unikraft/unikraft/lib/vfscore/include -I/root/.unikraft/unikraft/lib/devfs/include -I/root/.unikraft/unikraft/lib/uklock/include -I/root/.unikraft/unikraft/lib/uklibparam/include -I/root/.unikraft/unikraft/lib/uktime/include -I/root/.unikraft/unikraft/lib/uktime/musl-imported/include -I/root/.unikraft/unikraft/lib/posix-process/include -I/root/.unikraft/unikraft/lib/posix-process/musl-imported/include -I/root/.unikraft/unikraft/lib/posix-process/musl-imported/arch/generic -I/root/.unikraft/unikraft/lib/uksp/include -I/root/.unikraft/unikraft/lib/uksignal/include -I/root/.unikraft/apps/flexos-example/build/libsodium/origin/libsodium-1.0.18/src/libsodium/include -I/root/.unikraft/libs/newlib/include -I/root/.unikraft/libs/newlib/musl-imported/include -I/root/.unikraft/libs/newlib/musl-imported/arch/generic -I/root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include -I/root/.unikraft/apps/flexos-example/build/libcompiler_rt/origin/compiler-rt-1.7.0/compiler-rt/lib -I/root/.unikraft/apps/flexos-example/build/libcompiler_rt/origin/compiler-rt-1.7.0/compiler-rt/lib/builtins     -D__ARM_64__  -DCC_VERSION=13.0  -target aarch64-none-elf -g0 -march=morello+c64 -mabi=purecap --sysroot=/root/alistair/llvm1/llvm-project-releases/aarch64-none-elf+morello+c64+purecap/lib/ -DMISSING_SYSCALL_NAMES -DMALLOC_PROVIDED -D_POSIX_REALTIME_SIGNALS -Wno-char-subscripts -D__DYNAMIC_REENT__ -DCONFIG_UK_NETDEV_SCRATCH_SIZE=0   -Wno-unused-parameter    -g3 -D__LIBNAME__=libcompiler_rt -D__BASENAME__=divxc3.c -c /root/.unikraft/apps/flexos-example/build/libcompiler_rt/origin/compiler-rt-1.7.0/compiler-rt/lib/builtins/divxc3.c -o /root/.unikraft/apps/flexos-example/build/libcompiler_rt/divxc3.o -Wp,-MD,/root/.unikraft/apps/flexos-example/build/libcompiler_rt/.divxc3.o.d

source_/root/.unikraft/apps/flexos-example/build/libcompiler_rt/divxc3.o := /root/.unikraft/apps/flexos-example/build/libcompiler_rt/origin/compiler-rt-1.7.0/compiler-rt/lib/builtins/divxc3.c

deps_/root/.unikraft/apps/flexos-example/build/libcompiler_rt/divxc3.o := \
  /root/.unikraft/apps/flexos-example/build/libcompiler_rt/origin/compiler-rt-1.7.0/compiler-rt/lib/builtins/int_lib.h \
  /root/.unikraft/libs/newlib/include/float.h \
  /root/llvm1/llvm-project-releases/lib/clang/13.0.0/include/float.h \
  /root/.unikraft/libs/newlib/include/limits.h \
  /root/.unikraft/unikraft/include/uk/arch/limits.h \
  /root/.unikraft/unikraft/include/uk/config.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/limits.h \
    $(wildcard include/config/stack/size/page/order.h) \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/intsizes.h \
  /root/.unikraft/libs/newlib/include/stdbool.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/stdint.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/_default_types.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/features.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/_newlib_version.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_intsup.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_stdint.h \
  /root/.unikraft/apps/flexos-example/build/libcompiler_rt/origin/compiler-rt-1.7.0/compiler-rt/lib/builtins/int_types.h \
  /root/.unikraft/apps/flexos-example/build/libcompiler_rt/origin/compiler-rt-1.7.0/compiler-rt/lib/builtins/int_endianness.h \
  /root/.unikraft/apps/flexos-example/build/libcompiler_rt/origin/compiler-rt-1.7.0/compiler-rt/lib/builtins/int_util.h \
  /root/.unikraft/apps/flexos-example/build/libcompiler_rt/origin/compiler-rt-1.7.0/compiler-rt/lib/builtins/int_math.h \

/root/.unikraft/apps/flexos-example/build/libcompiler_rt/divxc3.o: $(deps_/root/.unikraft/apps/flexos-example/build/libcompiler_rt/divxc3.o)

$(deps_/root/.unikraft/apps/flexos-example/build/libcompiler_rt/divxc3.o):
