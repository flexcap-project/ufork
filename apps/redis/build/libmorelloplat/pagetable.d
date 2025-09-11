cmd_/root/.unikraft/apps/flexos-example/build/libmorelloplat/pagetable.o := ~/llvm1/llvm-project-releases/bin/clang -nostdlib -U __linux__ -U __FreeBSD__ -U __sun__ -fno-omit-frame-pointer -Werror=attributes -Wno-unused-variable -fpie -fno-stack-protector -Wall -Wextra -D __Unikraft__ -DUK_CODENAME="Tethys" -DUK_VERSION=0.5 -DUK_FULLVERSION=0.5.0~cceea60d-custom -O2   -I/root/.unikraft/apps/flexos-example/build/include -I/root/.unikraft/unikraft/arch/arm/arm64/include -I/root/.unikraft/unikraft/include -I/root/.unikraft/unikraft/lib/uklibparam/include  -I/root/.unikraft/unikraft/plat/morello/include -I/root/.unikraft/unikraft/plat/common/include   -D__ARM_64__  -DCC_VERSION=13.0  -target aarch64-none-elf  -g0 -march=morello+c64 -mabi=purecap -D__ASSEMBLY__ -mllvm -asm-macro-max-nesting-depth=1000  -DMORELLOPLAT -fpie     -g3 -D__LIBNAME__=libmorelloplat -D__BASENAME__=pagetable.S -c /root/.unikraft/unikraft/plat/morello/pagetable.S -o /root/.unikraft/apps/flexos-example/build/libmorelloplat/pagetable.o -Wp,-MD,/root/.unikraft/apps/flexos-example/build/libmorelloplat/.pagetable.o.d

source_/root/.unikraft/apps/flexos-example/build/libmorelloplat/pagetable.o := /root/.unikraft/unikraft/plat/morello/pagetable.S

deps_/root/.unikraft/apps/flexos-example/build/libmorelloplat/pagetable.o := \
    $(wildcard include/config/ukplat/lcpu/multicore.h) \
  /root/.unikraft/unikraft/include/uk/arch/limits.h \
  /root/.unikraft/unikraft/include/uk/config.h \
  /root/.unikraft/unikraft/arch/arm/arm64/include/uk/asm/limits.h \
    $(wildcard include/config/stack/size/page/order.h) \
  /root/.unikraft/unikraft/include/uk/asm.h \
  /root/.unikraft/unikraft/plat/morello/include/morello/mm.h \
  /root/.unikraft/unikraft/plat/common/include/arm/cpu_defs.h \
  /root/.unikraft/unikraft/plat/common/include/arm/arm64/cpu_defs.h \

/root/.unikraft/apps/flexos-example/build/libmorelloplat/pagetable.o: $(deps_/root/.unikraft/apps/flexos-example/build/libmorelloplat/pagetable.o)

$(deps_/root/.unikraft/apps/flexos-example/build/libmorelloplat/pagetable.o):
