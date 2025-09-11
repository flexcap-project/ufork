#define __SYSCALL_LL_E(x) (x)
#define __SYSCALL_LL_O(x) (x)

#define __asm_syscall(...) do { \
	__asm__ __volatile__ ( "svc 0" \
	: "=C"(x0) : __VA_ARGS__ : "memory", "cc"); \
	return x0; \
	} while (0)

static inline intptr_t __syscall0(intptr_t n)
{
	register intptr_t x8 __asm__("c8") = n;
	register intptr_t x0 __asm__("c0");
	__asm_syscall("C"(x8));
}

static inline intptr_t __syscall1(intptr_t n, intptr_t a)
{
	register intptr_t x8 __asm__("c8") = n;
	register intptr_t x0 __asm__("c0") = a;
	__asm_syscall("C"(x8), "0"(x0));
}

static inline intptr_t __syscall2(intptr_t n, intptr_t a, intptr_t b)
{
	register intptr_t x8 __asm__("c8") = n;
	register intptr_t x0 __asm__("c0") = a;
	register intptr_t x1 __asm__("c1") = b;
	__asm_syscall("C"(x8), "0"(x0), "C"(x1));
}

static inline intptr_t __syscall3(intptr_t n, intptr_t a, intptr_t b, intptr_t c)
{
	register intptr_t x8 __asm__("c8") = n;
	register intptr_t x0 __asm__("c0") = a;
	register intptr_t x1 __asm__("c1") = b;
	register intptr_t x2 __asm__("c2") = c;
	__asm_syscall("C"(x8), "0"(x0), "C"(x1), "C"(x2));
}

static inline intptr_t __syscall4(intptr_t n, intptr_t a, intptr_t b, intptr_t c, intptr_t d)
{
	register intptr_t x8 __asm__("c8") = n;
	register intptr_t x0 __asm__("c0") = a;
	register intptr_t x1 __asm__("c1") = b;
	register intptr_t x2 __asm__("c2") = c;
	register intptr_t x3 __asm__("c3") = d;
	__asm_syscall("C"(x8), "0"(x0), "C"(x1), "C"(x2), "C"(x3));
}

static inline intptr_t __syscall5(intptr_t n, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e)
{
	register intptr_t x8 __asm__("c8") = n;
	register intptr_t x0 __asm__("c0") = a;
	register intptr_t x1 __asm__("c1") = b;
	register intptr_t x2 __asm__("c2") = c;
	register intptr_t x3 __asm__("c3") = d;
	register intptr_t x4 __asm__("c4") = e;
	__asm_syscall("C"(x8), "0"(x0), "C"(x1), "C"(x2), "C"(x3), "C"(x4));
}

static inline intptr_t __syscall6(intptr_t n, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f)
{
	register intptr_t x8 __asm__("c8") = n;
	register intptr_t x0 __asm__("c0") = a;
	register intptr_t x1 __asm__("c1") = b;
	register intptr_t x2 __asm__("c2") = c;
	register intptr_t x3 __asm__("c3") = d;
	register intptr_t x4 __asm__("c4") = e;
	register intptr_t x5 __asm__("c5") = f;
	__asm_syscall("C"(x8), "0"(x0), "C"(x1), "C"(x2), "C"(x3), "C"(x4), "C"(x5));
}

#define VDSO_USEFUL
#define VDSO_CGT_SYM "__kernel_clock_gettime"
#define VDSO_CGT_VER "LINUX_2.6.39"

#define IPC_64 0
