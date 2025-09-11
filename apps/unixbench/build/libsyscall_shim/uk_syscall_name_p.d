cmd_/root/.unikraft/apps/flexos-example/build/libsyscall_shim/uk_syscall_name_p.o := ~/llvm1/llvm-project-releases/bin/clang -nostdlib -U __linux__ -U __FreeBSD__ -U __sun__ -fno-omit-frame-pointer -Werror=attributes -Wno-unused-variable -fno-stack-protector -Wall -Wextra -D __Unikraft__ -DUK_CODENAME="Tethys" -DUK_VERSION=0.5 -DUK_FULLVERSION=0.5.0~2dfa1ed-custom -O2 -fno-builtin -fno-PIC   -I/root/.unikraft/apps/flexos-example/build/include -I/root/.unikraft/unikraft/arch/arm/arm64/include -I/root/.unikraft/unikraft/include -I/root/.unikraft/unikraft/lib/flexos-core/include/ -I/root/.unikraft/unikraft/lib/ukboot/include -I/root/.unikraft/unikraft/lib/ukswrand/include -I/root/.unikraft/unikraft/lib/posix-user/include -I/root/.unikraft/unikraft/lib/posix-user/musl-imported/include -I/root/.unikraft/unikraft/lib/posix-sysinfo/include -I/root/.unikraft/unikraft/lib/ukdebug/include -I/root/.unikraft/unikraft/lib/ukargparse/include -I/root/.unikraft/unikraft/lib/nolibc/include -I/root/.unikraft/unikraft/lib/nolibc/arch/arm64 -I/root/.unikraft/unikraft/lib/nolibc/musl-imported/include -I/root/.unikraft/unikraft/lib/nolibc/musl-imported/arch/generic -I/root/.unikraft/unikraft/lib/ukalloc/include -I/root/.unikraft/unikraft/lib/ukallocregion/include -I/root/.unikraft/unikraft/lib/uksched/include -I/root/.unikraft/unikraft/lib/ukschedcoop/include -I/root/.unikraft/apps/flexos-example/build/libsyscall_shim/include -I/root/.unikraft/unikraft/lib/syscall_shim/include -I/root/.unikraft/unikraft/lib/vfscore/include -I/root/.unikraft/unikraft/lib/devfs/include -I/root/.unikraft/unikraft/lib/uklibparam/include -I/root/.unikraft/unikraft/lib/uktime/include -I/root/.unikraft/unikraft/lib/uktime/musl-imported/include -I/root/.unikraft/unikraft/lib/posix-process/include -I/root/.unikraft/unikraft/lib/posix-process/musl-imported/include -I/root/.unikraft/unikraft/lib/posix-process/musl-imported/arch/generic -I/root/.unikraft/unikraft/lib/uksp/include -I/root/.unikraft/unikraft/lib/uksignal/include -I/root/.unikraft/unikraft/lib/syscall_shim    -D__ARM_64__  -DCC_VERSION=13.0  -target aarch64-none-elf -g0 -march=morello+c64 -mabi=purecap --sysroot=/root/alistair/llvm1/llvm-project-releases/aarch64-none-elf+morello+c64+purecap/lib/ -fno-builtin-printf -fno-builtin-fprintf -fno-builtin-sprintf -fno-builtin-snprintf -fno-builtin-vprintf -fno-builtin-vfprintf -fno-builtin-vsprintf -fno-builtin-vsnprintf -fno-builtin-scanf -fno-builtin-fscanf -fno-builtin-sscanf -fno-builtin-vscanf -fno-builtin-vfscanf -fno-builtin-vsscanf -DCONFIG_UK_NETDEV_SCRATCH_SIZE=0       -g3 -D__LIBNAME__=libsyscall_shim -D__BASENAME__=uk_syscall_name_p.c -c /root/.unikraft/apps/flexos-example/build/libsyscall_shim/uk_syscall_name_p.c -o /root/.unikraft/apps/flexos-example/build/libsyscall_shim/uk_syscall_name_p.o -Wp,-MD,/root/.unikraft/apps/flexos-example/build/libsyscall_shim/.uk_syscall_name_p.o.d

source_/root/.unikraft/apps/flexos-example/build/libsyscall_shim/uk_syscall_name_p.o := /root/.unikraft/apps/flexos-example/build/libsyscall_shim/uk_syscall_name_p.c

deps_/root/.unikraft/apps/flexos-example/build/libsyscall_shim/uk_syscall_name_p.o := \
  /root/.unikraft/unikraft/lib/nolibc/include/stddef.h \
  /root/.unikraft/unikraft/include/uk/arch/types.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/intsizes.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/types.h \
  /root/.unikraft/unikraft/lib/nolibc/include/nolibc-internal/shareddefs.h \
    $(wildcard include/config/have/time.h) \
  /root/.unikraft/unikraft/include/uk/config.h \
  /root/.unikraft/unikraft/lib/uktime/include/uk/time_types.h \
  /root/.unikraft/unikraft/lib/syscall_shim/include/uk/syscall.h \
    $(wildcard include/config/libsyscall/shim.h) \
    $(wildcard include/config/libsyscall/shim/nowrapper.h) \
  /root/.unikraft/unikraft/include/uk/essentials.h \
    $(wildcard include/config/libnewlibc.h) \
    $(wildcard include/config/have/sched.h) \
  /root/.unikraft/unikraft/include/uk/errptr.h \
  /root/.unikraft/unikraft/lib/nolibc/include/errno.h \
  /root/.unikraft/unikraft/lib/nolibc/include/stdarg.h \
  /root/.unikraft/unikraft/lib/ukdebug/include/uk/print.h \
    $(wildcard include/config/libukdebug/printd.h) \
    $(wildcard include/config/libukdebug/printk/crit.h) \
    $(wildcard include/config/libukdebug/printk/err.h) \
    $(wildcard include/config/libukdebug/printk/warn.h) \
    $(wildcard include/config/libukdebug/printk/info.h) \
    $(wildcard include/config/libukdebug/printk.h) \
  /root/.unikraft/unikraft/lib/flexos-core/include/flexos/literals.h \
  /root/.unikraft/apps/flexos-example/build/libsyscall_shim/include/uk/bits/syscall_nrs.h \
  /root/.unikraft/apps/flexos-example/build/libsyscall_shim/include/uk/bits/syscall_map.h \
  /root/.unikraft/apps/flexos-example/build/libsyscall_shim/include/uk/bits/provided_syscalls.h \
  /root/.unikraft/apps/flexos-example/build/libsyscall_shim/include/uk/bits/syscall_stubs.h \

/root/.unikraft/apps/flexos-example/build/libsyscall_shim/uk_syscall_name_p.o: $(deps_/root/.unikraft/apps/flexos-example/build/libsyscall_shim/uk_syscall_name_p.o)

$(deps_/root/.unikraft/apps/flexos-example/build/libsyscall_shim/uk_syscall_name_p.o):
