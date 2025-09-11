cmd_/root/.unikraft/apps/flexos-example/build/libnewlibc/strnlen.o := ~/llvm1/llvm-project-releases/bin/clang -nostdlib -U __linux__ -U __FreeBSD__ -U __sun__ -fno-omit-frame-pointer -Werror=attributes -Wno-unused-variable -fno-stack-protector -Wall -Wextra -D __Unikraft__ -DUK_CODENAME="Tethys" -DUK_VERSION=0.5 -DUK_FULLVERSION=0.5.0~2dfa1ed-custom -O2 -fno-builtin -fno-PIC   -I/root/.unikraft/apps/flexos-example/build/include -I/root/.unikraft/unikraft/arch/arm/arm64/include -I/root/.unikraft/unikraft/include -I/root/.unikraft/unikraft/lib/uklibparam/include     -D__ARM_64__  -DCC_VERSION=13.0  -target aarch64-none-elf  -g0 -march=morello+c64 -mabi=purecap -D__ASSEMBLY__        -g3 -D__LIBNAME__=libnewlibc -D__BASENAME__=strnlen.S -c /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/machine/aarch64/strnlen.S -o /root/.unikraft/apps/flexos-example/build/libnewlibc/strnlen.o -Wp,-MD,/root/.unikraft/apps/flexos-example/build/libnewlibc/.strnlen.o.d

source_/root/.unikraft/apps/flexos-example/build/libnewlibc/strnlen.o := /root/.unikraft/apps/flexos-example/build/libnewlibc/origin/newlib-morello-release-1.7/newlib/libc/machine/aarch64/strnlen.S

deps_/root/.unikraft/apps/flexos-example/build/libnewlibc/strnlen.o := \

/root/.unikraft/apps/flexos-example/build/libnewlibc/strnlen.o: $(deps_/root/.unikraft/apps/flexos-example/build/libnewlibc/strnlen.o)

$(deps_/root/.unikraft/apps/flexos-example/build/libnewlibc/strnlen.o):
