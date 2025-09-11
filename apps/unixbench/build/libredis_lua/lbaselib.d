cmd_/root/.unikraft/apps/flexos-example/build/libredis_lua/lbaselib.o := ~/llvm1/llvm-project-releases/bin/clang -nostdlib -U __linux__ -U __FreeBSD__ -U __sun__ -fno-omit-frame-pointer -Werror=attributes -Wno-unused-variable -fpie -fno-stack-protector -Wall -Wextra -D __Unikraft__ -DUK_CODENAME="Tethys" -DUK_VERSION=0.5 -DUK_FULLVERSION=0.5.0~445952e9-custom -O2   -I/root/.unikraft/apps/flexos-example/build/include -I/root/.unikraft/unikraft/arch/arm/arm64/include -I/root/.unikraft/unikraft/include -I/root/.unikraft/unikraft/lib/flexos-core/include/ -I/root/.unikraft/unikraft/lib/sassy-fork/include/ -I/root/.unikraft/unikraft/lib/ukboot/include -I/root/.unikraft/unikraft/lib/ukswrand/include -I/root/.unikraft/unikraft/lib/posix-user/include -I/root/.unikraft/unikraft/lib/posix-user/musl-imported/include -I/root/.unikraft/unikraft/lib/posix-sysinfo/include -I/root/.unikraft/unikraft/lib/ukdebug/include -I/root/.unikraft/unikraft/lib/ukargparse/include -I/root/.unikraft/unikraft/lib/ukalloc/include -I/root/.unikraft/unikraft/lib/uksched/include -I/root/.unikraft/unikraft/lib/ukschedcoop/include -I/root/.unikraft/unikraft/lib/fdt/include -I/root/.unikraft/unikraft/lib/syscall_shim/include -I/root/.unikraft/unikraft/lib/vfscore/include -I/root/.unikraft/unikraft/lib/devfs/include -I/root/.unikraft/unikraft/lib/uklock/include -I/root/.unikraft/unikraft/lib/ukmpi/include -I/root/.unikraft/unikraft/lib/ukbus/include -I/root/.unikraft/unikraft/lib/uksglist/include -I/root/.unikraft/unikraft/lib/uknetdev/include -I/root/.unikraft/unikraft/lib/posix-libdl/include -I/root/.unikraft/unikraft/lib/uklibparam/include -I/root/.unikraft/unikraft/lib/uktime/include -I/root/.unikraft/unikraft/lib/uktime/musl-imported/include -I/root/.unikraft/unikraft/lib/ukblkdev/include -I/root/.unikraft/unikraft/lib/posix-process/include -I/root/.unikraft/unikraft/lib/posix-process/musl-imported/include -I/root/.unikraft/unikraft/lib/posix-process/musl-imported/arch/generic -I/root/.unikraft/unikraft/lib/uksp/include -I/root/.unikraft/unikraft/lib/uksignal/include -I/root/.unikraft/unikraft/lib/posix-mmap/include -I/root/.unikraft/libs/pthread-embedded/include -I/root/.unikraft/apps/flexos-example/build/libpthread-embedded/origin/pthread-embedded-44b41d760a433915d70a7be9809651b0a65e001d -I/root/.unikraft/apps/flexos-example/build/libpthread-embedded/origin/pthread-embedded-44b41d760a433915d70a7be9809651b0a65e001d/platform/helper -I/root/.unikraft/libs/lwip/include -I/root/.unikraft/libs/lwip/musl-imported/include -I/root/.unikraft/apps/flexos-example/build/liblwip/origin/lwip-2.1.2/src/include -I/root/.unikraft/libs/newlib/include -I/root/.unikraft/libs/newlib/musl-imported/include -I/root/.unikraft/libs/newlib/musl-imported/arch/generic -I/root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include -I/root/.unikraft/apps/flexos-example/build/libcompiler_rt/origin/compiler-rt-1.7.0/compiler-rt/lib -I/root/.unikraft/apps/flexos-example/build/libcompiler_rt/origin/compiler-rt-1.7.0/compiler-rt/lib/builtins -I/root/.unikraft/libs/lib-tinyalloc -I/root/.unikraft/libs/lib-tinyalloc/include -I/root/.unikraft/apps/flexos-example/build/libtinyalloc/origin/tinyalloc-96450f32d80fe7d23f6aa5426046143e57801bc4 -I/root/.unikraft/libs/redis/include     -D__ARM_64__  -DCC_VERSION=13.0  -target aarch64-none-elf -g0 -march=morello+c64 -mabi=purecap --sysroot=/root/alistair/llvm1/llvm-project-releases/aarch64-none-elf+morello+c64+purecap/lib/ -fomit-frame-pointer -g -DMISSING_SYSCALL_NAMES -DMALLOC_PROVIDED -D_POSIX_REALTIME_SIGNALS -D_WANT_IO_C99_FORMATS -D__LINUX_ERRNO_EXTENSIONS__ -D_LDBL_EQ_DBL -D_HAVE_LONG_DOUBLE -Wno-char-subscripts -D__DYNAMIC_REENT__ -DCONFIG_UK_NETDEV_SCRATCH_SIZE=4096   -Wno-unused-parameter -Wno-unused-variable -Wno-unused-value -Wno-implicit-fallthrough -Wno-char-subscripts -Wno-misleading-indentation  -DLUA_ANSI -DENABLE_CJSON_GLOBAL -DREDIS_STATIC=    -g3 -D__LIBNAME__=libredis_lua -D__BASENAME__=lbaselib.c -c /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/deps/lua/src/lbaselib.c -o /root/.unikraft/apps/flexos-example/build/libredis_lua/lbaselib.o -Wp,-MD,/root/.unikraft/apps/flexos-example/build/libredis_lua/.lbaselib.o.d

source_/root/.unikraft/apps/flexos-example/build/libredis_lua/lbaselib.o := /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/deps/lua/src/lbaselib.c

deps_/root/.unikraft/apps/flexos-example/build/libredis_lua/lbaselib.o := \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/ctype.h \
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
  /root/.unikraft/libs/newlib/include/stdio.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/stdio.h \
  /root/.unikraft/libs/newlib/include/stdarg.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/reent.h \
  /root/.unikraft/libs/newlib/include/sys/_types.h \
  /root/.unikraft/libs/newlib/include/uk/_types.h \
  /root/.unikraft/unikraft/lib/uktime/include/uk/time_types.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/stdint.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_intsup.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_stdint.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_types.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/_types.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/lock.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/types.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/types.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/select.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_sigset.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/_timeval.h \
  /root/.unikraft/libs/newlib/include/sys/timespec.h \
  /root/.unikraft/libs/newlib/include/uk/_timespec.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/stdio.h \
  /root/.unikraft/libs/newlib/include/stdlib.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/stdlib.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/machine/stdlib.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/alloca.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/string.h \
  /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/include/sys/string.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/deps/lua/src/lua.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/deps/lua/src/luaconf.h \
  /root/.unikraft/libs/newlib/include/limits.h \
  /root/.unikraft/unikraft/include/uk/arch/limits.h \
  /root/.unikraft/unikraft/include/uk/config.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/limits.h \
    $(wildcard include/config/stack/size/page/order.h) \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/intsizes.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/deps/lua/src/lauxlib.h \
  /root/.unikraft/apps/flexos-example/build/libredis/origin/redis-5.0.6/deps/lua/src/lualib.h \

/root/.unikraft/apps/flexos-example/build/libredis_lua/lbaselib.o: $(deps_/root/.unikraft/apps/flexos-example/build/libredis_lua/lbaselib.o)

$(deps_/root/.unikraft/apps/flexos-example/build/libredis_lua/lbaselib.o):
