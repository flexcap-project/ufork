#ifndef SASSYFORK_H
#define SASSYFORK_H

#include <sys/resource.h>
#include <uk/arch/spinlock.h>

#define ADVANCE_POINTER(increment) (increment / 8)
#define ADVANCE_CAPABILITY(increment) (increment / 16)
#define MAX_PROCESSES 30
#define PROCESS_PERM_MASK 

#define PROCESS_ID()					\
({	volatile unsigned long* val;						\
	__asm__ __volatile__("mrs %0, cid_el0\n isb\n"	\
			: "=r" (val));				\
	val;							\
})

#define PROCESS_LOCAL_STORAGE()					\
({	volatile unsigned long* val;						\
	__asm__ __volatile__("mrs %0, ctpidr_el0\n isb\n"	\
			: "=r" (val));				\
	val;							\
})
#define SP()					\
({	unsigned long* val;						\
	__asm__ __volatile__("mov %0, csp\n"	\
			: "=r" (val));				\
	val;							\
})

#define C_REG()					\
({	unsigned long* val;						\
	__asm__ __volatile__("mov %0, c22\n"	\
			: "=r" (val));				\
	val;							\
})

#define LR()					\
({	unsigned long* val;						\
	__asm__ __volatile__("mov %0, clr\n"	\
			: "=r" (val));				\
	val;							\
})

#define DO_SFORK()					\
({									\
	int pid;						\
	__asm__ __volatile__(			\
		"stp c1, c2, [csp, #-32]\n"	\
		"stp c3, c4, [csp, #-64]\n"	\
		"stp c5, c6, [csp, #-96]\n"	\
		"stp c7, c8, [csp, #-128]\n"	\
		"stp c9, c10, [csp, #-160]\n"	\
		"stp c11, c12, [csp, #-192]\n"	\
		"stp c13, c14, [csp, #-224]\n"	\
		"stp c15, c16, [csp, #-256]\n"	\
		"stp c17, c18, [csp, #-288]\n"	\
		"stp c19, c20, [csp, #-320]\n"	\
		"stp c21, c22, [csp, #-352]\n"	\
		"stp c23, c24, [csp, #-384]\n"	\
		"stp c25, c26, [csp, #-416]\n"	\
		"stp c27, c28, [csp, #-448]\n"	\
		"stp c29, c30, [csp, #-480]\n"	\
		"sub csp, csp, #480\n"			\
		"mov c0, csp\n"					\
		"bl sfork_jump_to\n"					\
		"ldp c29, c30, [csp]\n"			\
        "ldp c27, c28, [csp, #32]\n"	\
        "ldp c25, c26, [csp, #64]\n"	\
        "ldp c23, c24, [csp, #96]\n"	\
        "ldp c21, c22, [csp, #128]\n"	\
        "ldp c19, c20, [csp, #160]\n"	\
        "ldp c17, c18, [csp, #192]\n"	\
        "ldp c15, c16, [csp, #224]\n"	\
        "ldp c13, c14, [csp, #256]\n"	\
        "ldp c11, c12, [csp, #288]\n"	\
        "ldp c9, c10, [csp, #320]\n"	\
        "ldp c7, c8, [csp, #352]\n"		\
        "ldp c5, c6, [csp, #384]\n"		\
        "ldp c3, c4, [csp, #416]\n"		\
        "ldp c1, c2, [csp, #448]\n"		\
		"add csp, csp, #480\n"			\
		"mov %0, x0\n"					\
		:"=r"(pid) :  :"x0");					\
		pid;							\
})

struct sfork_arg {
	void *func;
	void *sp;
	void *tls;
	int pid;
	int ppid;
};

struct sfork_process {
	void** cap;
	void** rcap;
	int ppid;
	int state;
	int retcode;
	int occupied;
	int children[MAX_PROCESSES];
	int processor;
};

struct sfork_processor {
	void* csp;
	void* ptr;
	struct uk_thread* thread;
	int run;
	int wait;
};

extern char _start_kernel[], _start_app_copy[], _start_app_nocopy[], _end_kernel[], _binary_start[], __stop_binary[], _start_app_copy[], _start_got[], _end_got[];
extern uint32_t __page_refs[];
extern uint32_t *page_refs;
extern int __page_refs_size;
extern struct sfork_processor processors[];

struct syscalls_ptrs {
	int (*open)(char*, int, ...);
	int (*openat)(int, char*, int, ...);
	long (*write)(int, void*, size_t);
	void* (*malloc)(size_t);
	void (*free)(void*);
	void* (*memcpy)(void*, void*, size_t);
	void* (*memset)(void*, int, size_t);
	void* (*memalign)(size_t, size_t);
	size_t (*strlen)(char*);
	char* (*strerror)(int);
	int (*strcmp)(char*, char*);
	int (*gettimeofday)(void*, void*);
	int (*getpid)();
	int (*getppid)();
	int (*getpagesize)();
	int (*uname)(char*);
	long (*sysconf)(int);
	int (*getrlimit)(int, void*);
	void (*srandom)(int);
	void (*srand)(int);
	char* (*getenv)(char*);
	int (*getsockname)(int, void*, uint32_t*);
	int (*getsockopt)(int, int, int, void*, uint32_t*);
	int (*setsockopt)(int, int, int, const void*, uint32_t);
	long (*time)(void*);
	void* (*localtime)(void*);
	size_t (*strftime)(char*, size_t, char*, void*);
	int (*gethostname)(char*, size_t);
	int (*fstat)(int, void*);
	int (*lstat)(char*, void*);
	int (*stat)(char*, void*);
	int (*pread)(int, void*, size_t, long);
	int (*read)(int, void*, size_t);
	int (*close)(int);
	int (*mkdir)(char*, int);
	int (*rmdir)(char*);
	int (*chown)(char*, int, int);
	int (*chmod)(char*, int);
	int (*fcntl)(int, int, ...);
	int (*listen)(int, int);
	int (*socket)(int, int, int);
	int (*bind)(int, void*, int);
	int (*ioctl)(int, long, ...);
	int (*sigemptyset)(void*);
	int (*sigaddset)(void*, int);
	int (*sigprocmask)(int, void*, void*);
	int (*sigsuspend)(void*);
	int (*sigaction)(int, void*, void*);
	int (*pwrite)(int, void*, size_t, long);
	int (*dup2)(int, int);
	int (*setitimer)(long, void*, void*);
	int (*socketpair)(int, int, int, int*);
	int (*sfork)(void*, long);
	ssize_t (*sendmsg)(int, void*, int);
	int (*setpriority)(int, int, int);
	int (*setrlimit)(int, void*);
	int (*geteuid)();
	int (*setgid)(int);
	int (*initgroups)(char*, int);
	int (*setuid)(int);
	int (*prctl)(int, ...);
	int (*chdir)(char*);
	int (*printf)(char*, ...);
	int (*printf1)(char*);
	ssize_t (*recvmsg)(int, void*, int);
	void* (*signal)(int, void*);
	long (*lseek)(int, long, int);
	int (*fsync)(int);
	void (*sfork_exit)(int);
	int (*sfork_wait3)(int*, void*, int);

	int (*poll)(void*, unsigned int, int);
	int (*connect)(int, void*, uint32_t);
	int (*recv)(int, void*, size_t, int);
	int (*recvfrom)(int, void*, size_t, int, void*, void*);
	int (*send)(int, void*, size_t, int);
	int (*sendto)(int, void*, size_t, int, void*, long);
	int (*ftruncate)(int, long);
	char* (*getcwd)(char*, size_t);
	int (*statvfs)(char*, void*);
	int (*statfs)(char*, void*);
	int (*unlink)(char*);
	int (*rename)(char*, char*);
	int (*fstatat)(int, char*, void*, int);
	ssize_t (*writev)(int, void*, int);
	ssize_t (*readv)(int, void*, int);
	int (*accept)(int, void*, void*);
	int (*clock_gettime)(int, void*);
	int (*utime)(char*, void*);
	int (*select)(int, void*, void*, void*, void*);
	int (*erra)();
	void (*errb)(int);
	void (*tzset)();
	int* (*err)();
	void (*asser)(const char*, int, const char*);
	char* (*setlocale)(int, const char*);
	void* (*fopen)(const char*, const char*);
	void* (*calloc)(size_t, size_t);
	void* (*realloc)(void*, size_t);
	int (*pipe)(int*);
	unsigned long (*irqf_save)();
	void (*irqf_restore)(unsigned long);
	char* (*fgets)(char*, int, void*);
	void* (*rent)();
	int (*getaddrinfo)(const char*, const char*, const void*, void**);
	void (*freeaddrinfo)(void*);
	void* (*sched_default)();
	void* (*sched_thread_create)(void*, const char*, const void*, void (void*), void*);
	void (*thread_inherit_signal_mask)(void*);
	int (*isatty)(int);
	int (*epoll_create)(int);
	int (*epoll_ctl)(int, int, int, void*);
	int (*coutk)(const char*, unsigned int);
	void (*sched_thread_sleep)(unsigned long);
	int (*ptsigmask)(int, const void*, void*);
	void (*tblockt)(void*, unsigned long);
	unsigned long (*monclock)();
	void (*yield)();


};

extern struct syscalls_ptrs sfork_syscalls;

#define SHARED_MAPPING_BEGIN _start_kernel
#define SHARED_MAPPING_END _end_kernel
#define TOTAL_SIZE (unsigned long)__stop_binary - (unsigned long)_binary_start
#define PADDING_BETWEEN_PROCESSES 10 * PAGE_SIZE
#define COPYABLE_PAGES ((unsigned long)__stop_binary - (unsigned long)_start_app_copy)/PAGE_SIZE

#define TOTAL_PAGES 131071 // big hack, this is the number of pages present between 0xe0000000 - 0xffffffff, update when we have more memory

extern unsigned long start_points[];
extern unsigned long ptb;
extern unsigned long ptb1;
extern unsigned long end_points[];
extern unsigned long page_references[];
extern int proc_slot_occupied[];
extern int process_parent[];
extern void** process_capability[];
extern void* global_cap;
extern struct uk_alloc* proc_allocators[];
extern __spinlock lock2 __attribute__((section(".kernel_data")));
extern __spinlock plock1;
extern int sfork_err;
extern uintptr_t* got_start;

int copy_page(unsigned long** source_page, unsigned long** dest);
unsigned long new_process();
int register_process(unsigned long pid, unsigned long start_address, unsigned long end_address);
unsigned long offset_address_this_process(unsigned long vaddr, unsigned long cpid);
unsigned long offset_address_process(unsigned long vaddr, unsigned long ppid, unsigned long cpid);
int sfork_init();
void sfork_exit(int retcode);
int copy_pages(unsigned long** start, unsigned long** end, unsigned long** dest);
void* move_capability(void* cap, unsigned long new_base, unsigned long pid);
void *adjust_stack(unsigned long sp, int cpid, int ppid);
int sfork_wait3(int *wstatus, int options, struct rusage *rusage);
int sfork(void* sp, unsigned long lr);
int sfork_jump_to(void* sp)  __attribute__((section(".app_text")));
void handle_level3_page_fault(unsigned long far, unsigned long elr, int sc);
void wait_for_inst();
int handle_irq();
void wait_proc();
int enter_main_proc0(unsigned long ptr, int argc, void* argv);
void* sfork_copy_from_user(void* ptr, size_t len);
void* sfork_copy_to_user(void* ptr, void* pptr, size_t len);





char *getenv_kernel(const char *name);
int uname_kernel( struct utsname * buf);
int gettimeofday_kernel(struct timeval *tv, void *tz __unused);
int clock_gettime_kernel(clockid_t clk_id, struct timespec* tp);
long sysconf_kernel(int name);
int setpriority_kernel(int which, id_t who, int prio);
int epoll_create_kernel(int size);




#endif
