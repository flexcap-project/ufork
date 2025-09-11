cmd_/root/.unikraft/apps/micropython/build/libvfscore/libparam.lds := ~/llvm1/llvm-project-releases/bin/clang -E -P -x assembler-with-cpp -nostdlib -U __linux__ -U __FreeBSD__ -U __sun__ -fno-omit-frame-pointer -Werror=attributes -Wno-unused-variable -fpie -fno-stack-protector -Wall -Wextra -D __Unikraft__ -DUK_CODENAME="Tethys" -DUK_VERSION=0.5 -DUK_FULLVERSION=0.5.0~f2d81b1b-custom -O2 -I/root/.unikraft/apps/micropython/build/include -I/root/.unikraft/unikraft/arch/arm/arm64/include -I/root/.unikraft/unikraft/include -I/root/.unikraft/unikraft/lib/uklibparam/include     -D__ARM_64__   -DCC_VERSION=13.0  -target aarch64-none-elf  -g0 -march=morello+c64 -mabi=purecap -D__ASSEMBLY__ -mllvm -asm-macro-max-nesting-depth=1000  -DUK_LIBPARAM_PREFIX=vfs    /root/.unikraft/unikraft/lib/uklibparam/libparam.lds.S -o /root/.unikraft/apps/micropython/build/libvfscore/libparam.lds -Wp,-MD,/root/.unikraft/apps/micropython/build/libvfscore/.libparam.lds.d

source_/root/.unikraft/apps/micropython/build/libvfscore/libparam.lds := /root/.unikraft/unikraft/lib/uklibparam/libparam.lds.S

deps_/root/.unikraft/apps/micropython/build/libvfscore/libparam.lds := \
  /root/.unikraft/unikraft/include/uk/config.h \
  /root/.unikraft/unikraft/lib/uklibparam/include/uk/libparam.h \
    $(wildcard include/config/libuklibparam.h) \

/root/.unikraft/apps/micropython/build/libvfscore/libparam.lds: $(deps_/root/.unikraft/apps/micropython/build/libvfscore/libparam.lds)

$(deps_/root/.unikraft/apps/micropython/build/libvfscore/libparam.lds):
