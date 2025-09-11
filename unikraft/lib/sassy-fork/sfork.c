#include <sfork.h>
#include <uk/plat/mm.h>
#include <uk/assert.h>
#include <flexos/isolation.h>
#include <pthread.h> 
#include <sched.h>
#include <uk/sched.h>
#include <uk/thread.h>
#include <uk/plat/config.h>
#include <uk/plat/time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/socket.h>
#include <signal.h>
#include <grp.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/vfs.h>
#include <utime.h>
#include <uk/uk_signal.h>
#include <arpa/inet.h>
#include <vfscore/eventpoll.h>
#include <uk/plat/console.h>


unsigned long start_points[MAX_PROCESSES];
unsigned long application_start_points[MAX_PROCESSES];
unsigned long end_points[MAX_PROCESSES];
unsigned long page_references[TOTAL_PAGES]; //use page_refs instead
 //-1 no process, 0 stopped, 1 active
struct sfork_process processes[MAX_PROCESSES];
void* global_cap;
uint32_t *page_refs;
struct uk_alloc* proc_allocators[MAX_PROCESSES] __attribute__((section(".kernel_data")));
struct sfork_processor processors[4] __attribute__((section(".kernel_data")));
struct sfork_processor first[4] __attribute__((section(".kernel_data")));
unsigned long ptb;
unsigned long ptb1;
 struct syscalls_ptrs sfork_syscalls __attribute__((section(".app_data")));
int sfork_err __attribute__((section(".app_data")));
uintptr_t* got_start = 0;

static __spinlock lock __attribute__((section(".kernel_data")));
static __spinlock lock1 __attribute__((section(".kernel_data")));
__spinlock lock2 __attribute__((section(".kernel_data")));
 __spinlock plock1;
volatile int processor_pf = -1;
static int go = 0;

int prev_core = 0;
int same_core = 0;

struct sfork_arg sargs[MAX_PROCESSES] __attribute__((section(".kernel_data")));

static inline void _tlb_flush(void)
{
	__asm__ __volatile__(
		"	dsb	ishst\n"     /* wait for write complete */
		"	tlbi	vmalle1is\n" /* invalidate all */
		"	dsb	ish\n"       /* wait for invalidate complete */
		"	dsb sy\n"
		"	isb\n"               /* sync context */
		::: "memory");
}

static inline void ukarch_spin_lock_sfork(struct __spinlock *lock)
{
	register int r, locked = 1;

	__asm__ __volatile__(
		"	sevl\n"			/* set event locally */
		"1:	wfe\n"			/* wait for event */
		"2:	ldaxr	%w0, [%1]\n"	/* exclusive load lock value */
		"	cbz	    %w0, 3f\n"	/* check if already locked */
        "   ldxr    %w0, [%3]\n"
        "   subs    %w0, %w0, %w4\n"
        "   cbnz    %w0, 1b\n"
        "   cbz     %w0, 4f\n"
		"3:	stxr	%w0, %2, [%1]\n"/* try to lock it */
		"	cbnz	%w0, 2b\n"	/* jump to l2 if we failed */
        "   stxr	%w0, %w4, [%3]\n"
        "4:\n"
		: "=&r" (r)
		: "r" (&lock->lock), "r" (locked), "r"(&processor_pf), "r"(ukplat_lcpu_id()));
}

static inline void ukarch_spin_unlock_sfork(struct __spinlock *lock)
{
	__asm__ __volatile__(
        "stxr   %w1, %w1, [%2]\n"
		"stlr	wzr, [%0]\n"		/* unlock lock */
		"sev\n"				/* wake up any waiters */
		:
		: "r" (&lock->lock), "r"(-1), "r"(&processor_pf));
}

static inline void ukarch_tlb_flush(void)
{
	__asm__ __volatile__(
		"	dsb	ishst\n"     /* wait for write complete */
		"	tlbi	vmalle1is\n" /* invalidate all */
        "TLBI VMALLE1\n"
        "DSB ISHST\n"
        "TLBI VMALLE1\n"
		"	dsb	ish\n"       /* wait for invalidate complete */
        "   dsb st\n"
		"	dsb sy\n"
		"	isb\n"               /* sync context */
		::: "memory");
}


int handle_irq() {
   //unused

    return 1;
}

void wait_for_inst() {
    if (ukplat_lcpu_id() == 0)
    enable_irq();
    processors[ukplat_lcpu_id()].run = 0;
	while(processors[ukplat_lcpu_id()].run == 0) 
    {
    #ifdef CONFIG_SFORK_COW
        asm("sevl\n");
    #endif
    }
    
    ukarch_tlb_flush();
    UK_ASSERT(processors[ukplat_lcpu_id()].thread != NULL);
    struct uk_thread* thread = processors[ukplat_lcpu_id()].thread;
    
    uk_sched_get_default()->plat_ctx_cbs.start_cb(thread->ctx);
}

void* sfork_copy_from_user(void* ptr, size_t len) {
    void* kptr = uk_malloc(k_alloc, len);
    memcpy(kptr, ptr, len);
    return kptr;
}

void* sfork_copy_to_user(void* ptr, void* pptr, size_t len) {
    memcpy(pptr, ptr, len);
    uk_free(k_alloc, ptr);
    return pptr;
}

int addr_to_page_ref(unsigned long addr, int pid) {
    unsigned long app_start = application_start_points[pid];
    unsigned long diff = PAGE_ALIGN_DOWN(addr) - app_start;
    UK_ASSERT(diff >= 0);

    int index = diff / PAGE_SIZE;
    return index * (pid+1);
}

void add_page_ref(int index, int pid) {
    (page_refs[index]) |= (1 << pid);
}

void remove_page_ref(int index, int pid) {
    (page_refs[index]) &= ~((uint32_t)1 << pid);
}

uint32_t get_page_ref(int index) {
    return (page_refs[index]);
}

int page_has_ref(int index, int pid){
    UK_ASSERT(__builtin_cheri_tag_get((page_refs)) == 1);
    return (((page_refs[index])) & (1 << pid)) >> pid;
}


//because this is very unstable prototype software, eventually this needs to be made nice,
//but for now I had no time for niceness
// also many of these tlb flushes etc are probably overkill and hurting performance bigly
// again, this is a rough and ready PoC
#ifdef CONFIG_SFORK_SMP

void handle_level3_page_fault(unsigned long far, unsigned long elr, int sc) {

#if defined CONFIG_LIBSASSYFORK_SMP || CONFIG_LIBSASSYFORK_COA

    ukarch_spin_lock(&lock);
        __asm__(
        "ldr x0, [%0]\n"
        "msr ttbr0_el1, x0\n"
        "isb\n"
        :
        : "r"(&ptb1)
        :"x0", "x1"

    );
    ukarch_tlb_flush();

#endif

    int process_id = PROCESS_ID(); // current process 
    int ppid = processes[process_id].ppid;

    int proc = ukplat_lcpu_id(); //processor

    unsigned long page_aligned_far = PAGE_ALIGN_DOWN(far);
    unsigned long* page_aligned_far_cap = int_to_cap(page_aligned_far);

    unsigned long* mapped_to_page = pt_pte_to_cap(get_pte(page_aligned_far), 3); //which page are we mapped to?


    int far_pid = find_pid(far);
    int mapped_to_pid = find_pid(mapped_to_page);

    if (far_pid == mapped_to_pid && far_pid > 0)
        goto out_handler;
    

    int mapped_to_page_index = addr_to_page_ref(mapped_to_page, mapped_to_pid);
    if (mapped_to_pid == 0 && far_pid > 0) {
    
        unsigned long* child_page = int_to_cap(offset_address_process(mapped_to_page, 0, far_pid));
   
        uk_page_map(mapped_to_page, mapped_to_page, PAGE_ATTR_PROT_NONE, 0); //break
        ukarch_tlb_flush();

        
        copy_page(mapped_to_page, child_page);
        uk_page_map(child_page, child_page, PAGE_ATTR_PROT_WRITE, 0);
        remove_page_ref(mapped_to_page_index, far_pid);

        
        if (page_refs[mapped_to_page_index] == 0) {
            uk_page_map(mapped_to_page, mapped_to_page, PAGE_ATTR_PROT_WRITE, 0);
        }
        goto out_handler;
      } else if (far_pid == 0) {
        
        for (int i = 1; i < 4; i++) {
            if (page_has_ref(mapped_to_page_index, i)) {
                 //parent RO for this
                unsigned long* child_page = int_to_cap(offset_address_process(mapped_to_page, 0, i));
                
                uk_page_map(child_page, child_page, PAGE_ATTR_PROT_NONE, 0);
            }
        }
        ukarch_tlb_flush();
     
        for (int i = 1; i < 4; i++) {
            if (page_has_ref(mapped_to_page_index, i)) {
                 //parent RO for this
                unsigned long* child_page = int_to_cap(offset_address_process(mapped_to_page, 0, i));
                copy_page(mapped_to_page, child_page);
                uk_page_map(child_page, child_page, PAGE_ATTR_PROT_WRITE, 0);
                remove_page_ref(mapped_to_page_index, i);
            }
        }
        uk_page_map(mapped_to_page, mapped_to_page, PAGE_ATTR_PROT_WRITE, 0);
 
        ukarch_tlb_flush();
        goto out_handler;

     }

    
    out_handler:;
     
                __asm__(
        "ldr x0, [%0]\n"
        "msr ttbr0_el1, x0\n"
        "isb\n"
        "dsb st\n"
        :
        : "r"(&ptb)
        :"x0", "x1"

    );
    ukarch_tlb_flush();

    ukarch_spin_unlock(&lock);

}



#else

void handle_level3_page_fault(unsigned long far, unsigned long elr, int sc) {

#if defined CONFIG_LIBSASSYFORK_SMP || CONFIG_LIBSASSYFORK_COA
    ukarch_spin_lock(&lock);
        __asm__(
        "ldr x0, [%0]\n"
        "msr ttbr0_el1, x0\n"
        "isb\n"
        :
        : "r"(&ptb1)
        :"x0", "x1"

    );

#endif

    int process_id = PROCESS_ID(); // current process 
    int ppid = processes[process_id].ppid;

    int proc = ukplat_lcpu_id();

    unsigned long page_aligned_far = PAGE_ALIGN_DOWN(far);
    unsigned long* page_aligned_far_cap = int_to_cap(page_aligned_far);

    unsigned long* mapped_to_page = pt_pte_to_cap(get_pte(page_aligned_far), 3);

    int far_pid = find_pid(far);
    int mapped_to_pid = find_pid(mapped_to_page);
      
    uk_page_map(page_aligned_far_cap, page_aligned_far_cap, PAGE_ATTR_PROT_WRITE, 0);
    if ((ppid > -1) && (ppid == far_pid)) {

        int current_index = addr_to_page_ref(mapped_to_page, mapped_to_pid);
        for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[mapped_to_pid].children[i] > -1) {
            if (page_has_ref(current_index, i) == 1) {
                unsigned long* child_page = int_to_cap(offset_address_process(mapped_to_page, mapped_to_pid, i));
            #ifdef CONFIG_LIBSASSYFORK_SMP
                copy_page(mapped_to_page, child_page);
                uk_page_map(child_page, child_page, PAGE_ATTR_PROT_WRITE, 0);
            #else
                uk_page_map(child_page, child_page, PAGE_ATTR_PROT_WRITE, 0);
                copy_page(mapped_to_page, child_page);
            #endif
                remove_page_ref(current_index, i);
                int child_index = addr_to_page_ref(child_page, i);
                if (get_page_ref(child_index)) {
                #ifdef CONFIG_LIBSASSYFORK_COA
                    uk_page_set_prot(child_page, PAGE_ATTR_PROT_NONE);
                #else
                    uk_page_set_prot(child_page, PAGE_ATTR_PROT_READ);
                #endif
                }
            }
        }
    }
        uint32_t parent_page_ref = get_page_ref(current_index);

        if (!parent_page_ref) {
            uk_page_map(mapped_to_page, mapped_to_page, PAGE_ATTR_PROT_WRITE, 0);

        }

    } else if (ppid > -1 && ppid != far_pid && far_pid == process_id) {
        int current_index = addr_to_page_ref(mapped_to_page, mapped_to_pid);
        for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[mapped_to_pid].children[i] > -1) {
            if (page_has_ref(current_index, i) == 1) {
                unsigned long* child_page = int_to_cap(offset_address_process(mapped_to_page, mapped_to_pid, i));
            #ifdef CONFIG_LIBSASSYFORK_SMP
                copy_page(mapped_to_page, child_page);
                uk_page_map(child_page, child_page, PAGE_ATTR_PROT_WRITE, 0);
            #else
                uk_page_map(child_page, child_page, PAGE_ATTR_PROT_WRITE, 0);
                copy_page(mapped_to_page, child_page);
            #endif
                remove_page_ref(current_index, i);
                int child_index = addr_to_page_ref(child_page, i);
                if (get_page_ref(child_index)) {
                #ifdef CONFIG_LIBSASSYFORK_COA
                    uk_page_set_prot(child_page, PAGE_ATTR_PROT_NONE);
                #else
                    uk_page_set_prot(child_page, PAGE_ATTR_PROT_READ);
                #endif 
                }
            }
        }
    }
 
        uint32_t parent_page_ref = get_page_ref(current_index);

        if (!parent_page_ref) {
            // set the page to rw
            uk_page_map(mapped_to_page, mapped_to_page, PAGE_ATTR_PROT_WRITE, 0);
        }
        

    }
    int current_index = addr_to_page_ref(far, process_id);
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[process_id].children[i] > -1) {
            if (page_has_ref(current_index, i) == 1) {
                unsigned long* child_page = int_to_cap(offset_address_process(page_aligned_far, process_id, i));
            #ifdef CONFIG_LIBSASSYFORK_SMP
                copy_page(mapped_to_page, child_page);
                uk_page_map(child_page, child_page, PAGE_ATTR_PROT_WRITE, 0);
            #else
                uk_page_map(child_page, child_page, PAGE_ATTR_PROT_WRITE, 0);
                copy_page(mapped_to_page, child_page);
                
            #endif
                remove_page_ref(current_index, i);
                int child_index = addr_to_page_ref(child_page, i);
                if (get_page_ref(child_index)) {
                #ifdef CONFIG_LIBSASSYFORK_COA
                    uk_page_set_prot(child_page, PAGE_ATTR_PROT_NONE);
                #else
                    uk_page_set_prot(child_page, PAGE_ATTR_PROT_READ);
                #endif
                }
            }
        }
    }

#ifdef CONFIG_LIBSASSYFORK_SMP
        if (process_id == 0) {
        int current_index = addr_to_page_ref(far, process_id);
            if (!get_page_ref(current_index)) {
                uk_page_set_prot(page_aligned_far, PAGE_ATTR_PROT_WRITE);

            }
            
        }
#endif    
    out_handler:;
#if defined CONFIG_LIBSASSYFORK_SMP || CONFIG_LIBSASSYFORK_COA
        
    __asm__(
        "ldr x0, [%0]\n"
        "msr ttbr0_el1, x0\n"
        "isb\n"
        "dsb st\n"
        :
        : "r"(&ptb)
        :"x0", "x1"

    );
    ukarch_tlb_flush();

#endif

    ukarch_spin_unlock(&lock);




}

#endif

int find_pid(unsigned long addr) {
    int pid = -1;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (addr >= start_points[i] && addr < end_points[i])
            return i;
    }

    return pid; //we couldn't find a matching process, could be kernel
}

void* move_capability(void* cap, unsigned long new_base, unsigned long pid) {
    if (new_base >= offset_address_process(_start_app_nocopy, 0, pid)) {
        void* rcap = processes[pid].rcap;
        rcap = __builtin_cheri_offset_set(rcap, new_base - __builtin_cheri_base_get(rcap));
        UK_ASSERT(__builtin_cheri_tag_get(rcap) == 1);
        return rcap;
    }
    long length = __builtin_cheri_length_get(cap);
    void* proc_cap = (void*)processes[pid].cap;
    UK_ASSERT(__builtin_cheri_tag_get(proc_cap));
    void* new_cap = __builtin_cheri_address_set(proc_cap, new_base);
    UK_ASSERT(__builtin_cheri_tag_get(new_cap));
    if (length != -1 && length < __builtin_cheri_length_get(proc_cap) && length+__builtin_cheri_address_get(new_cap) < end_points[pid]) {
        UK_ASSERT(__builtin_cheri_tag_get(new_cap));
        new_cap = __builtin_cheri_bounds_set(new_cap, length);
    }
    UK_ASSERT(__builtin_cheri_tag_get(new_cap) == 1);
    return new_cap;
}



unsigned long offset_address_this_process(unsigned long vaddr_in_parent, unsigned long cpid) {
    unsigned long this_pid = PROCESS_ID();
    UK_ASSERT(start_points[cpid] != 0);
    UK_ASSERT(start_points[this_pid] != 0);
    unsigned long offset_between_start_points = start_points[cpid] > start_points[this_pid] ? 
                        start_points[cpid] - start_points[this_pid] : start_points[this_pid] - start_points[cpid];

    return vaddr_in_parent + offset_between_start_points;
}

unsigned long offset_address_process(unsigned long vaddr_in_parent, unsigned long ppid, unsigned long cpid) {
    UK_ASSERT(start_points[ppid] != 0);
    UK_ASSERT(start_points[cpid] != 0);

    if (ppid == cpid)
        return vaddr_in_parent;

    unsigned long offset_between_start_points = start_points[cpid] > start_points[ppid] ? 
                        start_points[cpid] - start_points[ppid] : start_points[ppid] - start_points[cpid];

    return vaddr_in_parent + offset_between_start_points;
}

void *adjust_stack(unsigned long sp, int cpid, int ppid) {
    return int_to_cap(offset_address_process(sp, ppid, cpid));
}

int copy_page(unsigned long** source_page, unsigned long** dest) {
    /* Pages need to be page aligned */
    UK_ASSERT(PAGE_ALIGNED((unsigned long)source_page));
    UK_ASSERT(PAGE_ALIGNED((unsigned long)dest));


    int src_pid = find_pid((unsigned long)source_page);
    if (src_pid < 0) {
        //UK_CRASH("Not in a process, maybe we shouldn't be copying from this, source page %p\n", source_page);
        return 0;
    }

    int dest_pid = find_pid((unsigned long)dest);
    if (dest_pid < 0) {
        //UK_CRASH("Not in a process, maybe we shouldn't be copying to this, dest page %p\n", dest);
        return 0;
    }

    unsigned long before = *(source_page);

    size_t locs = 0;
    if (memcpy(dest, source_page, PAGE_SIZE) < 1) {
        return 1;
    }

    unsigned long after = *(dest);

    /* Scan the page for any capabilities which need updating */
    /* TO DO, even if the capabilities are not pointing to the wrong process, we may want to restrict bounds anyway*/
    unsigned long** source1 = source_page;
    for (unsigned long** offset = dest; (unsigned long)offset < dest + ADVANCE_CAPABILITY(PAGE_SIZE); offset++) {
        if (__builtin_cheri_tag_get(*(offset))) {
                unsigned long* one = *offset;
         unsigned long* two = *source1;
 
            if (__builtin_cheri_address_get(*(offset)) > start_points[src_pid] && __builtin_cheri_address_get(*(offset)) < end_points[src_pid]) {
                void** thing = move_capability(*(offset), offset_address_process(__builtin_cheri_address_get(*(offset)), src_pid, dest_pid), dest_pid);
                UK_ASSERT(__builtin_cheri_tag_get(thing));
                *offset = thing;
                UK_ASSERT(__builtin_cheri_tag_get(*offset));   
            } 
        }
        source1++;
        
    }


    return 0;
}


int copy_page_app(unsigned long** source_page, unsigned long** dest) {

    /* Pages need to be page aligned */
    UK_ASSERT(PAGE_ALIGNED((unsigned long)source_page));
    UK_ASSERT(PAGE_ALIGNED((unsigned long)dest));

    int src_pid = find_pid((unsigned long)source_page);
    if (src_pid < 0) {
        //UK_CRASH("Not in a process, maybe we shouldn't be copying from this, source page %p\n", source_page);
        return 0;
    }

    int dest_pid = find_pid((unsigned long)dest);
    if (dest_pid < 0) {
        //UK_CRASH("Not in a process, maybe we shouldn't be copying to this, dest page %p\n", dest);
        return 0;
    }

    unsigned long before = *(source_page);

    size_t locs = 0;
    if (memcpy(dest, source_page, PAGE_SIZE) < 1) {
        return 1;
    }

    unsigned long after = *(dest);

    /* Scan the page for any capabilities which need updating */
    /* TO DO, even if the capabilities are not pointing to the wrong process, we may want to restrict bounds anyway*/
    unsigned long** source1 = source_page;
    for (unsigned long** offset = dest; (unsigned long)offset < dest + ADVANCE_CAPABILITY(PAGE_SIZE); offset++) {
        if (__builtin_cheri_tag_get(*(offset))) {
                unsigned long* one = *offset;
         unsigned long* two = *source1;
            if (__builtin_cheri_address_get(*(offset)) >= application_start_points[src_pid] && __builtin_cheri_address_get(*(offset)) < end_points[src_pid]) {
                void** thing = move_capability(*(offset), offset_address_process(__builtin_cheri_address_get(*(offset)), src_pid, dest_pid), dest_pid);
                UK_ASSERT(__builtin_cheri_tag_get(thing));
                *offset = thing;
                UK_ASSERT(__builtin_cheri_tag_get(*offset));   
            } 
        }
        source1++;
        
    }


    return 0;
}


int remove_process(unsigned long pid) {
    processes[pid].occupied = 0;
    processes[pid].ppid = -2;
    processes[pid].retcode = -1;
    processes[pid].state = -1;
    /* We probably want to clean memory here */
    return 0;
} 

unsigned long next_available_proc_slot() {
    for (long l = 0; l < MAX_PROCESSES; l++) {
        if (!processes[l].occupied) {
            processes[l].occupied = 1;
            return l;
        }
    }
    return -1; // Reached max processes and no slots are free
}

void setup_page_references(int pid, int ppid) {
    UK_ASSERT(pid != ppid);

    if (ppid >= 0) {
        for (int i = 0; i < COPYABLE_PAGES; i++) {
            (page_refs[i*(ppid+1)]) |= (1 << pid);
        }
    }
}

void setup_root_page_references() {
    setup_page_references(0, -1);
}

void remove_page_references(int pid, int ppid) {
    UK_ASSERT(pid != ppid);
    if (ppid >= 0) {
        for (int i = 0; i < (end_points[0] - application_start_points[0])/PAGE_SIZE; i++) {
            (page_refs[i*(ppid+1)]) &= ~((uint32_t)1 << pid);
        }
    }
}


/*
*   Call this when we want a new process
*   Function creates a new process
*   Registers next available start and end points
*   @return new pid
*
*/
unsigned long new_process() {
    unsigned long new_pid = next_available_proc_slot();
    UK_ASSERT(new_pid >= 0);
    unsigned long ppid = (unsigned long)PROCESS_ID();
    processes[new_pid].ppid = ppid;
    processes[ppid].children[new_pid] = 1;
    return new_pid;
}

int init_start_and_end_points() {
    unsigned long size = TOTAL_SIZE;
    unsigned long kernel_size = (unsigned long)_start_app_copy - (unsigned long)_binary_start;
    unsigned long page_aligned_up_size = PAGE_ALIGN_UP(size);
    unsigned long padded_size = page_aligned_up_size + PADDING_BETWEEN_PROCESSES;
    UK_ASSERT(PAGE_ALIGNED(padded_size));

    unsigned long start = (unsigned long)_binary_start;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        start_points[i] = start;
        application_start_points[i] = start + kernel_size;
        end_points[i] = start + page_aligned_up_size;
        processes[i].occupied = 0;
        start += padded_size;
    }

    return 0;
}

int init_process_states() {

    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].state = -1;
        processes[i].retcode = -1;
        processes[i].ppid = -2;
        for (int j = 0; j < MAX_PROCESSES; j++) {
            processes[i].children[j] = -1;
        }
    }

    return 0;
}

int sfork_errno() {
    return errno;
}

void sfork_errno_set(int error) {
    errno = error;
}

int enter_main_proc0(unsigned long ptr, int argc, void* argv) {
    int (*root_cap)(int, void*) = processes[0].cap;
    root_cap = __builtin_cheri_address_set(root_cap, (unsigned long)got_start);
    root_cap = __builtin_cheri_bounds_set(root_cap, end_points[0] - (unsigned long)got_start);
    root_cap = __builtin_cheri_offset_set(root_cap, ptr - __builtin_cheri_base_get(root_cap));
    return root_cap(argc, argv);
}

void* seal_cap_rb(void * ptr) __attribute__((section(".app_text"))){
    void* ret = ptr;
    __asm__ __volatile__(
                    "ldr c0, [%0]\n"
                   "seal c0, c0, rb\n"
                   "str c0, [%0]\n"
			        :  : "r"(ret): "c0");

    return ret;
}

void init_syscalls() __attribute__((section(".app_text"))){
    sfork_syscalls.open = __builtin_cheri_offset_set(processes[0].cap, open_kernel - start_points[0]);
   
    sfork_syscalls.openat = __builtin_cheri_offset_set(processes[0].cap, openat - start_points[0]);
    
    sfork_syscalls.write = __builtin_cheri_offset_set(processes[0].cap, write_kernel - start_points[0]);
    
    sfork_syscalls.malloc = __builtin_cheri_offset_set(processes[0].cap, malloc_kernel - start_points[0]);
    
    sfork_syscalls.free = __builtin_cheri_offset_set(processes[0].cap, free_kernel - start_points[0]);
    
    sfork_syscalls.memcpy = __builtin_cheri_offset_set(processes[0].cap, memcpy - start_points[0]);
    
    sfork_syscalls.memset = __builtin_cheri_offset_set(processes[0].cap, memset - start_points[0]);
    
    sfork_syscalls.memalign = __builtin_cheri_offset_set(processes[0].cap, memalign_kernel - start_points[0]);
    
    sfork_syscalls.strlen = __builtin_cheri_offset_set(processes[0].cap, strlen - start_points[0]);
    
    sfork_syscalls.strerror = __builtin_cheri_offset_set(processes[0].cap, strerror - start_points[0]);
    
    sfork_syscalls.strcmp = __builtin_cheri_offset_set(processes[0].cap, strcmp - start_points[0]);
    
    sfork_syscalls.gettimeofday = __builtin_cheri_offset_set(processes[0].cap, gettimeofday_kernel - start_points[0]);
    
    sfork_syscalls.getpid = __builtin_cheri_offset_set(processes[0].cap, getpid - start_points[0]);
    
    sfork_syscalls.getppid = __builtin_cheri_offset_set(processes[0].cap, getppid - start_points[0]);
    
    sfork_syscalls.getpagesize = __builtin_cheri_offset_set(processes[0].cap, getpagesize - start_points[0]);
    
    sfork_syscalls.uname = __builtin_cheri_offset_set(processes[0].cap, uname_kernel - start_points[0]);
    
    sfork_syscalls.sysconf = __builtin_cheri_offset_set(processes[0].cap, sysconf_kernel - start_points[0]);
    
    sfork_syscalls.getrlimit = __builtin_cheri_offset_set(processes[0].cap, getrlimit - start_points[0]);
    
    sfork_syscalls.srandom = __builtin_cheri_offset_set(processes[0].cap, srandom_kernel - start_points[0]);

    sfork_syscalls.srand = __builtin_cheri_offset_set(processes[0].cap, srand_kernel - start_points[0]);

    sfork_syscalls.getenv = __builtin_cheri_offset_set(processes[0].cap, getenv_kernel - start_points[0]);

    sfork_syscalls.getsockname = __builtin_cheri_offset_set(processes[0].cap, getsockname_kernel - start_points[0]);

    sfork_syscalls.getsockopt = __builtin_cheri_offset_set(processes[0].cap, getsockopt_kernel - start_points[0]);

    sfork_syscalls.setsockopt = __builtin_cheri_offset_set(processes[0].cap, setsockopt_kernel - start_points[0]);

    sfork_syscalls.time = __builtin_cheri_offset_set(processes[0].cap, time_kernel - start_points[0]);

    sfork_syscalls.localtime = __builtin_cheri_offset_set(processes[0].cap, localtime_kernel - start_points[0]);

    sfork_syscalls.strftime = __builtin_cheri_offset_set(processes[0].cap, strftime - start_points[0]);

    sfork_syscalls.gethostname = __builtin_cheri_offset_set(processes[0].cap, gethostname_kernel - start_points[0]);

    sfork_syscalls.fstat = __builtin_cheri_offset_set(processes[0].cap, fstat_kernel - start_points[0]);

    sfork_syscalls.lstat = __builtin_cheri_offset_set(processes[0].cap, lstat_kernel - start_points[0]);

    sfork_syscalls.stat = __builtin_cheri_offset_set(processes[0].cap, stat_kernel - start_points[0]);

    sfork_syscalls.read = __builtin_cheri_offset_set(processes[0].cap, read_kernel - start_points[0]);

    sfork_syscalls.pread = __builtin_cheri_offset_set(processes[0].cap, pread_kernel - start_points[0]);

    sfork_syscalls.close = __builtin_cheri_offset_set(processes[0].cap, close_kernel - start_points[0]);

    sfork_syscalls.mkdir = __builtin_cheri_offset_set(processes[0].cap, mkdir_kernel - start_points[0]);

    sfork_syscalls.rmdir = __builtin_cheri_offset_set(processes[0].cap, rmdir_kernel - start_points[0]);

    sfork_syscalls.chown = __builtin_cheri_offset_set(processes[0].cap, chown - start_points[0]); //probs don't need

    sfork_syscalls.chmod = __builtin_cheri_offset_set(processes[0].cap, chmod - start_points[0]); //probs don't need

    sfork_syscalls.fcntl = __builtin_cheri_offset_set(processes[0].cap, fcntl_kernel - start_points[0]);

    sfork_syscalls.listen = __builtin_cheri_offset_set(processes[0].cap, listen_kernel - start_points[0]);

    sfork_syscalls.bind = __builtin_cheri_offset_set(processes[0].cap, bind_kernel - start_points[0]);

    sfork_syscalls.socket = __builtin_cheri_offset_set(processes[0].cap, socket_kernel - start_points[0]);

    sfork_syscalls.ioctl = __builtin_cheri_offset_set(processes[0].cap, ioctl_kernel - start_points[0]);

    sfork_syscalls.sigemptyset = __builtin_cheri_offset_set(processes[0].cap, sigemptyset_kernel - start_points[0]);

    sfork_syscalls.sigaddset = __builtin_cheri_offset_set(processes[0].cap, sigaddset_kernel - start_points[0]);

    sfork_syscalls.sigprocmask = __builtin_cheri_offset_set(processes[0].cap, sigprocmask_kernel - start_points[0]);

    sfork_syscalls.sigsuspend = __builtin_cheri_offset_set(processes[0].cap, sigsuspend_kernel - start_points[0]);

    sfork_syscalls.sigaction = __builtin_cheri_offset_set(processes[0].cap, sigaction_kernel - start_points[0]);

    sfork_syscalls.pwrite = __builtin_cheri_offset_set(processes[0].cap, pwrite_kernel - start_points[0]);

    sfork_syscalls.dup2 = __builtin_cheri_offset_set(processes[0].cap, dup2_kernel - start_points[0]);

    sfork_syscalls.setitimer = __builtin_cheri_offset_set(processes[0].cap, setitimer - start_points[0]);

    sfork_syscalls.socketpair = __builtin_cheri_offset_set(processes[0].cap, socketpair_kernel - start_points[0]);

    sfork_syscalls.sfork = __builtin_cheri_offset_set(processes[0].cap, sfork - start_points[0]);

    sfork_syscalls.sendmsg = __builtin_cheri_offset_set(processes[0].cap, sendmsg_kernel - start_points[0]);

    sfork_syscalls.setpriority = __builtin_cheri_offset_set(processes[0].cap, setpriority_kernel - start_points[0]);

    sfork_syscalls.setrlimit = __builtin_cheri_offset_set(processes[0].cap, setrlimit - start_points[0]);

    sfork_syscalls.geteuid = __builtin_cheri_offset_set(processes[0].cap, geteuid - start_points[0]);

    sfork_syscalls.setgid = __builtin_cheri_offset_set(processes[0].cap, setgid - start_points[0]);

    sfork_syscalls.initgroups = __builtin_cheri_offset_set(processes[0].cap, initgroups - start_points[0]);

    sfork_syscalls.setuid = __builtin_cheri_offset_set(processes[0].cap, setuid - start_points[0]);

    sfork_syscalls.prctl = __builtin_cheri_offset_set(processes[0].cap, prctl - start_points[0]);

    sfork_syscalls.chdir = __builtin_cheri_offset_set(processes[0].cap, chdir_kernel - start_points[0]);

    sfork_syscalls.printf = __builtin_cheri_offset_set(processes[0].cap, printf - start_points[0]);

    sfork_syscalls.printf1 = __builtin_cheri_offset_set(processes[0].cap, printf - start_points[0]);

    sfork_syscalls.recvmsg = __builtin_cheri_offset_set(processes[0].cap, recvmsg_kernel - start_points[0]);

    sfork_syscalls.signal = __builtin_cheri_offset_set(processes[0].cap, signal_kernel - start_points[0]);

    sfork_syscalls.lseek = __builtin_cheri_offset_set(processes[0].cap, lseek_kernel - start_points[0]);

    sfork_syscalls.fsync = __builtin_cheri_offset_set(processes[0].cap, fsync_kernel - start_points[0]);

    sfork_syscalls.sfork_exit = __builtin_cheri_offset_set(processes[0].cap, sfork_exit - start_points[0]);

    sfork_syscalls.sfork_wait3 = __builtin_cheri_offset_set(processes[0].cap, sfork_wait3 - start_points[0]);


    sfork_syscalls.poll = __builtin_cheri_offset_set(processes[0].cap, poll_kernel - start_points[0]);
    
    sfork_syscalls.connect = __builtin_cheri_offset_set(processes[0].cap, connect_kernel - start_points[0]);
    
    sfork_syscalls.recv = __builtin_cheri_offset_set(processes[0].cap, recv_kernel - start_points[0]);
    
    sfork_syscalls.recvfrom = __builtin_cheri_offset_set(processes[0].cap, recvfrom_kernel - start_points[0]);
    
    sfork_syscalls.send = __builtin_cheri_offset_set(processes[0].cap, send_kernel - start_points[0]);
    
    sfork_syscalls.sendto = __builtin_cheri_offset_set(processes[0].cap, sendto_kernel - start_points[0]);
    
    sfork_syscalls.ftruncate = __builtin_cheri_offset_set(processes[0].cap, ftruncate - start_points[0]);
    
    sfork_syscalls.getcwd = __builtin_cheri_offset_set(processes[0].cap, getcwd_kernel - start_points[0]);
    
    sfork_syscalls.statvfs = __builtin_cheri_offset_set(processes[0].cap, statvfs_kernel - start_points[0]);
    
    sfork_syscalls.statfs = __builtin_cheri_offset_set(processes[0].cap, __statfs_kernel - start_points[0]);
    
    sfork_syscalls.unlink = __builtin_cheri_offset_set(processes[0].cap, unlink_kernel - start_points[0]);
    
    sfork_syscalls.rename = __builtin_cheri_offset_set(processes[0].cap, rename_kernel - start_points[0]);
    
    sfork_syscalls.fstatat = __builtin_cheri_offset_set(processes[0].cap, fstatat_kernel - start_points[0]);
    
    sfork_syscalls.writev = __builtin_cheri_offset_set(processes[0].cap, writev_kernel - start_points[0]);
    
    sfork_syscalls.readv = __builtin_cheri_offset_set(processes[0].cap, readv_kernel - start_points[0]);
    
    sfork_syscalls.accept = __builtin_cheri_offset_set(processes[0].cap, accept_kernel - start_points[0]);
    
    sfork_syscalls.clock_gettime = __builtin_cheri_offset_set(processes[0].cap, clock_gettime_kernel - start_points[0]);
    
    sfork_syscalls.utime = __builtin_cheri_offset_set(processes[0].cap, utime - start_points[0]);
    
    sfork_syscalls.select = __builtin_cheri_offset_set(processes[0].cap, select_kernel - start_points[0]);
    
    sfork_syscalls.erra = __builtin_cheri_offset_set(processes[0].cap, sfork_errno - start_points[0]);
    
    sfork_syscalls.errb = __builtin_cheri_offset_set(processes[0].cap, sfork_errno_set - start_points[0]);
    sfork_syscalls.tzset = __builtin_cheri_offset_set(processes[0].cap, tzset - start_points[0]);
    
    sfork_syscalls.err = __builtin_cheri_offset_set(processes[0].cap, __errno_kernel - start_points[0]);
    
    sfork_syscalls.asser = __builtin_cheri_offset_set(processes[0].cap, __assert_kernel - start_points[0]);
    
    sfork_syscalls.setlocale = __builtin_cheri_offset_set(processes[0].cap, setlocale_kernel - start_points[0]);
    
    sfork_syscalls.fopen = __builtin_cheri_offset_set(processes[0].cap, fopen_kernel - start_points[0]);
    
    sfork_syscalls.calloc = __builtin_cheri_offset_set(processes[0].cap, calloc_kernel - start_points[0]);
    
    sfork_syscalls.realloc = __builtin_cheri_offset_set(processes[0].cap, realloc_kernel - start_points[0]);
    
    sfork_syscalls.pipe = __builtin_cheri_offset_set(processes[0].cap, pipe_kernel - start_points[0]);
    
    sfork_syscalls.irqf_save = __builtin_cheri_offset_set(processes[0].cap, ukplat_lcpu_save_irqf_kernel - start_points[0]);
    
    sfork_syscalls.irqf_restore = __builtin_cheri_offset_set(processes[0].cap, ukplat_lcpu_restore_irqf_kernel - start_points[0]);
    
    sfork_syscalls.fgets = __builtin_cheri_offset_set(processes[0].cap, fgets_kernel - start_points[0]);
    
    sfork_syscalls.rent = __builtin_cheri_offset_set(processes[0].cap, __getreent_kernel - start_points[0]);
    
    sfork_syscalls.getaddrinfo = __builtin_cheri_offset_set(processes[0].cap, getaddrinfo_kernel - start_points[0]);
    
    sfork_syscalls.freeaddrinfo = __builtin_cheri_offset_set(processes[0].cap, freeaddrinfo_kernel - start_points[0]);
    
    sfork_syscalls.sched_default = __builtin_cheri_offset_set(processes[0].cap, uk_sched_get_default_kernel - start_points[0]);
    
    sfork_syscalls.sched_thread_create = __builtin_cheri_offset_set(processes[0].cap, uk_sched_thread_create_kernel - start_points[0]);
    
    sfork_syscalls.thread_inherit_signal_mask = __builtin_cheri_offset_set(processes[0].cap, uk_thread_inherit_signal_mask_kernel - start_points[0]);
    
    sfork_syscalls.isatty = __builtin_cheri_offset_set(processes[0].cap, isatty_kernel - start_points[0]);
    
    sfork_syscalls.epoll_create = __builtin_cheri_offset_set(processes[0].cap, epoll_create_kernel - start_points[0]);
    
    sfork_syscalls.epoll_ctl = __builtin_cheri_offset_set(processes[0].cap, epoll_ctl_kernel - start_points[0]);
    
    sfork_syscalls.coutk = __builtin_cheri_offset_set(processes[0].cap, ukplat_coutk_kernel - start_points[0]);
    
    sfork_syscalls.sched_thread_sleep = __builtin_cheri_offset_set(processes[0].cap, uk_sched_thread_sleep_kernel - start_points[0]);
    
    sfork_syscalls.ptsigmask = __builtin_cheri_offset_set(processes[0].cap, pthread_sigmask - start_points[0]);
    
    sfork_syscalls.tblockt = __builtin_cheri_offset_set(processes[0].cap, uk_thread_block_timeout_kernel - start_points[0]);
    
    sfork_syscalls.monclock = __builtin_cheri_offset_set(processes[0].cap, ukplat_monotonic_clock_kernel - start_points[0]);
    
    sfork_syscalls.yield = __builtin_cheri_offset_set(processes[0].cap, uk_sched_yield - start_points[0]);
    
}


void init_process_root_capabilities() __attribute__((section(".app_text"))){

    unsigned long size = TOTAL_SIZE ;
    unsigned long page_aligned_up_size = PAGE_ALIGN_UP(size);

    /* AK TODO we'll want to restrict perms here too */
    for (int i = 0; i < MAX_PROCESSES; i++) {
        void *cap;
        void* rcap;
        __asm__ __volatile__(
        "cvtp %0, %1\n"
        "scvalue %0, %0, %1\n"
        "scbnds %0, %0, %2\n"
			: "=r" (cap) : "r"((unsigned long)start_points[i]), "r"(page_aligned_up_size) :);
        processes[i].cap = cap;
        rcap = __builtin_cheri_address_set(cap, (unsigned long)offset_address_process((unsigned long)got_start, 0, i));
        rcap = __builtin_cheri_bounds_set(rcap, end_points[0] - (unsigned long)got_start);
        processes[i].rcap = rcap;
        UK_ASSERT(__builtin_cheri_tag_get(processes[i].cap));
    }
}

void remove_system_permission() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].cap = __builtin_cheri_perms_and(processes[i].cap, ~((1 << 10)));;
    }
}

//This is horrible and messy and quick and dirty, please do better
// also yes I am aware there is a lot of code duplication, I left it for time reasons
void sfork_init_scan() __attribute__((section(".app_text"))){
    unsigned long start = application_start_points[0];
    unsigned long stop = end_points[0];
    unsigned long kernel_start = start_points[0];
    struct uk_thread *curr = uk_thread_current();
    unsigned  long sp_end = curr->stack;
    unsigned  long sp_start = curr->stack - STACK_SIZE;
    for (volatile void** i = int_to_cap(start); i <= stop; i++) {
        if (__builtin_cheri_tag_get(*i) == 1 && (i < sp_start || i > sp_end)) {

            volatile int sealed = __builtin_cheri_sealed_get(*i);
            //is capability
            unsigned long addr = __builtin_cheri_address_get(*i);
            unsigned long base = __builtin_cheri_base_get(*i);
             long length = __builtin_cheri_length_get(*i);
             int executable = (__builtin_cheri_perms_get(*i) & __CHERI_CAP_PERMISSION_PERMIT_EXECUTE__)  == 0 ? 1 : 0;
            if ((base + length > stop || length == -1) && addr >= application_start_points[0]) {
                
                unsigned long * new_cap = *i;
                if (base == 0) {

                    __asm__ __volatile__(
                    "cvtd c0, %1\n"
                    "scvalue c0, c0, %1\n"
                   "str c0, [%0]\n"
			        :  : "r"(&new_cap), "r"((unsigned long)application_start_points[0]), "r"(addr - application_start_points[0]):"c0");
                }
                
                if (length > stop - addr || length == -1) {
                                  __asm__ __volatile__(
                    "ldr c0, [%0]\n"
                    "scbnds c0, c0, %1\n"
                    "str c0, [%0]\n"
			        ::"r"(&new_cap), "r"((unsigned long)stop- application_start_points[0]) :"c0");
                }

                 __asm__ __volatile__(
                    "ldr c0, [%0]\n"
                   "scoff c0, c0, %2\n"
                   "str c0, [%0]\n"
			        :  : "r"(&new_cap), "r"((unsigned long)application_start_points), "r"(addr - application_start_points[0]):"c0");

                if (addr %2 != 0) {
                         __asm__ __volatile__(
                    "ldr c0, [%0]\n"
                   "seal c0, c0, rb\n"
                   "str c0, [%0]\n"
			        :  : "r"(&new_cap), "r"((unsigned long)kernel_start), "r"(addr - kernel_start):"c0");

                    }
                *i = new_cap;
            } 
            else if ((base + length > stop || length == -1) && addr < application_start_points[0]) {
                int executable = (__builtin_cheri_perms_get(*i) & __CHERI_CAP_PERMISSION_PERMIT_EXECUTE__)  == 0 ? 1 : 0;
                unsigned long * new_cap = *i;
                if (addr %2 == 0)
                if (base == 0) {

                __asm__ __volatile__(
                    "cvtd c0, %1\n"
                    "scvalue c0, c0, %1\n"
                    "str c0, [%0]\n"
			        :  : "r"(&new_cap), "r"((unsigned long)kernel_start), "r"(addr - kernel_start):"c0");
                }

                //FIXME
                
                if (addr %2 != 0) {
                                  __asm__ __volatile__(
                    "ldr c0, [%0]\n"
                    "scbnds c0, c0, %1\n"
                    "str c0, [%0]\n"
			        ::"r"(&new_cap), "r"((unsigned long)stop - kernel_start) :"c0");
                } else {
                                   __asm__ __volatile__(
                    "ldr c0, [%0]\n"
                    "scbnds c0, c0, %1\n"
                    "str c0, [%0]\n"
			        ::"r"(&new_cap), "r"((unsigned long)application_start_points[0] - kernel_start) :"c0");
                }

                 __asm__ __volatile__(
                    "ldr c0, [%0]\n"
                   "scoff c0, c0, %2\n"
                   "str c0, [%0]\n"
			        :  : "r"(&new_cap), "r"((unsigned long)kernel_start), "r"(addr - kernel_start):"c0");

  if (addr %2 != 0) {
                         __asm__ __volatile__(
                    "ldr c0, [%0]\n"
                   "seal c0, c0, rb\n"
                   "str c0, [%0]\n"
			        :  : "r"(&new_cap), "r"((unsigned long)kernel_start), "r"(addr - kernel_start):"c0");

                    }
                *i = new_cap;
            }

        }
    }
}

//... why app_text? .... don't even ask, there's something wierd going on with offsets between sections so here we are
void seal_syscalls() __attribute__((section(".app_text"))){
     __asm__ __volatile__(
                    "ldr c0, [%0]\n"
                   "seal c0, c0, rb\n"
                   "str c0, [%0]\n"
			        :  : "r"(&sfork_syscalls.open): "c0");
    __asm__ __volatile__(
                    "ldr c0, [%0]\n"
                   "seal c0, c0, rb\n"
                   "str c0, [%0]\n"
			        :  : "r"(&sfork_syscalls.openat): "c0");
    __asm__ __volatile__(
                    "ldr c0, [%0]\n"
                   "seal c0, c0, rb\n"
                   "str c0, [%0]\n"
			        :  : "r"(&sfork_syscalls.write): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.malloc): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.free): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.memcpy): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.memset): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.memalign): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.strlen): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.strerror): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.strcmp): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.gettimeofday): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.getpid): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.getppid): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.getpagesize): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.uname): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.sysconf): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.getrlimit): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.srandom): "c0");
        
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.srand): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.getenv): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.getsockname): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.getsockopt): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.setsockopt): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.time): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.localtime): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.strftime): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.gethostname): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.fstat): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.lstat): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.stat): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.read): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.pread): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.close): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.mkdir): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.rmdir): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.chown): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.chmod): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.fcntl): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.listen): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.bind): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.socket): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.ioctl): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.sigemptyset): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.sigaddset): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.sigprocmask): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.sigsuspend): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.sigaction): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.pwrite): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.dup2): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.setitimer): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.socketpair): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.sfork): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.sendmsg): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.setpriority): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.setrlimit): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.geteuid): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.setgid): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.initgroups): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.setuid): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.prctl): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.chdir): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.printf): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.printf1): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.recvmsg): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.signal): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.lseek): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.fsync): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.sfork_exit): "c0");
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.sfork_wait3): "c0");    

    // sfork_syscalls.poll = seal_cap_rb(&sfork_syscalls.poll);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.poll): "c0"); 
    // sfork_syscalls.connect = seal_cap_rb(&sfork_syscalls.connect);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.connect): "c0"); 
    // sfork_syscalls.recv = seal_cap_rb(&sfork_syscalls.recv);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.recv): "c0"); 
    // sfork_syscalls.recvfrom = seal_cap_rb(&sfork_syscalls.recvfrom);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.recvfrom): "c0"); 
    // sfork_syscalls.send = seal_cap_rb(&sfork_syscalls.send);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.send): "c0"); 
    // sfork_syscalls.sendto = seal_cap_rb(&sfork_syscalls.sendto);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.sendto): "c0"); 
    // sfork_syscalls.ftruncate = seal_cap_rb(&sfork_syscalls.ftruncate);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.ftruncate): "c0"); 
    // sfork_syscalls.getcwd = seal_cap_rb(&sfork_syscalls.getcwd);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.getcwd): "c0"); 
    // sfork_syscalls.statvfs = seal_cap_rb(&sfork_syscalls.statvfs);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.statvfs): "c0"); 
    // sfork_syscalls.statfs = seal_cap_rb(&sfork_syscalls.statfs);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.statfs): "c0"); 
    // sfork_syscalls.unlink = seal_cap_rb(&sfork_syscalls.unlink);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.unlink): "c0"); 
    // sfork_syscalls.rename = seal_cap_rb(&sfork_syscalls.rename);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.rename): "c0"); 
    // sfork_syscalls.fstatat = seal_cap_rb(&sfork_syscalls.fstatat);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.fstatat): "c0"); 
    // sfork_syscalls.writev = seal_cap_rb(&sfork_syscalls.writev);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.writev): "c0"); 
    // sfork_syscalls.readv = seal_cap_rb(&sfork_syscalls.readv);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.readv): "c0"); 
    // sfork_syscalls.accept = seal_cap_rb(&sfork_syscalls.accept);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.accept): "c0"); 
    // sfork_syscalls.clock_gettime = seal_cap_rb(&sfork_syscalls.clock_gettime);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.clock_gettime): "c0"); 
    // sfork_syscalls.utime = seal_cap_rb(&sfork_syscalls.utime);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.utime): "c0"); 
    // sfork_syscalls.select = seal_cap_rb(&sfork_syscalls.select);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.select): "c0"); 
    // sfork_syscalls.erra = seal_cap_rb(&sfork_syscalls.erra);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.erra): "c0"); 
    // sfork_syscalls.errb = seal_cap_rb(&sfork_syscalls.errb);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.errb): "c0"); 
    // sfork_syscalls.tzset = seal_cap_rb(&sfork_syscalls.tzset);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.tzset): "c0"); 
    // sfork_syscalls.err = seal_cap_rb(&sfork_syscalls.err);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.err): "c0"); 
    // sfork_syscalls.asser = seal_cap_rb(&sfork_syscalls.asser);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.asser): "c0"); 
    // sfork_syscalls.setlocale = seal_cap_rb(&sfork_syscalls.setlocale);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.setlocale): "c0"); 
    // sfork_syscalls.fopen = seal_cap_rb(&sfork_syscalls.fopen);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.fopen): "c0"); 
    // sfork_syscalls.calloc = seal_cap_rb(&sfork_syscalls.calloc);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.calloc): "c0"); 
    // sfork_syscalls.realloc = seal_cap_rb(&sfork_syscalls.realloc);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.realloc): "c0"); 
    // sfork_syscalls.pipe = seal_cap_rb(&sfork_syscalls.pipe);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.pipe): "c0"); 
    // sfork_syscalls.irqf_save = seal_cap_rb(&sfork_syscalls.irqf_save);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.irqf_save): "c0"); 
    // sfork_syscalls.irqf_restore = seal_cap_rb(&sfork_syscalls.irqf_restore);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.irqf_restore): "c0"); 
    // sfork_syscalls.fgets = seal_cap_rb(&sfork_syscalls.fgets);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.fgets): "c0"); 
    // sfork_syscalls.rent = seal_cap_rb(&sfork_syscalls.rent);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.rent): "c0"); 
    // sfork_syscalls.getaddrinfo = seal_cap_rb(&sfork_syscalls.getaddrinfo);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.getaddrinfo): "c0"); 
    // sfork_syscalls.freeaddrinfo = seal_cap_rb(&sfork_syscalls.freeaddrinfo);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.freeaddrinfo): "c0"); 
    // sfork_syscalls.sched_default = seal_cap_rb(&sfork_syscalls.sched_default);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.sched_default): "c0"); 
    // sfork_syscalls.sched_thread_create = seal_cap_rb(&sfork_syscalls.sched_thread_create);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.sched_thread_create): "c0"); 
    // sfork_syscalls.thread_inherit_signal_mask = seal_cap_rb(&sfork_syscalls.thread_inherit_signal_mask);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.thread_inherit_signal_mask): "c0"); 
    // sfork_syscalls.isatty = seal_cap_rb(&sfork_syscalls.isatty);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.isatty): "c0"); 
    // sfork_syscalls.epoll_create = seal_cap_rb(&sfork_syscalls.epoll_create);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.epoll_create): "c0"); 
    // sfork_syscalls.epoll_ctl = seal_cap_rb(&sfork_syscalls.epoll_ctl);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.epoll_ctl): "c0"); 
    // sfork_syscalls.coutk = seal_cap_rb(&sfork_syscalls.coutk);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.coutk): "c0"); 
    // sfork_syscalls.sched_thread_sleep = seal_cap_rb(&sfork_syscalls.sched_thread_sleep);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.sched_thread_sleep): "c0"); 
    // sfork_syscalls.ptsigmask = seal_cap_rb(&sfork_syscalls.ptsigmask);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.ptsigmask): "c0"); 
    // sfork_syscalls.tblockt = seal_cap_rb(&sfork_syscalls.tblockt);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.tblockt): "c0"); 
    // sfork_syscalls.monclock = seal_cap_rb(&sfork_syscalls.monclock);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.monclock): "c0"); 
    // sfork_syscalls.yield = seal_cap_rb(&sfork_syscalls.yield);
    __asm__ __volatile__(
                        "ldr c0, [%0]\n"
                    "seal c0, c0, rb\n"
                    "str c0, [%0]\n"
                        :  : "r"(&sfork_syscalls.yield): "c0"); 

}

int sfork_init() {
    /* Setup global capability */
	__asm__ __volatile__(
        "cvtp %0, %1\n"
			: "=r" (global_cap) : "r"((unsigned long)_binary_start) :);

    page_refs = int_to_cap((unsigned long)__page_refs);
    init_start_and_end_points();
    init_process_root_capabilities();
    init_process_states();
    next_available_proc_slot();
    processes[0].ppid = -1;
    processes[0].state = 1;
    setup_root_page_references();
    proc_allocators[0] = flexos_shared_alloc;
    processors[1].run = 0;
    processors[2].run = 0;
    processors[3].run = 0;
    processors[0].wait = 0;
    processors[1].wait = 0;
    processors[2].wait = 0;
    processors[3].wait = 0;
    first[1].run = 0;
    first[2].run = 0;
    first[3].run = 0;
    sfork_init_scan();
    init_syscalls();
    seal_syscalls();
    remove_system_permission();
    ukarch_spin_init(&lock);
    ukarch_spin_init(&lock1);
    ukarch_spin_init(&lock2);
    return 0;

}

int sfork_free_processor() {
    for (int i = 1; i < 4; i++) {
        if (processors[i].run == 0) {
            return i;
        }
    }
    return 0;
}

int copy_pages(unsigned long** start, unsigned long** end, unsigned long** dest) {
    for (long pages = 0; pages < (PAGE_ALIGN_UP((unsigned long)end - (unsigned long)start) / PAGE_SIZE); pages++) {
		UK_ASSERT(!copy_page(start + ADVANCE_CAPABILITY(pages * PAGE_SIZE), dest + ADVANCE_CAPABILITY(pages * PAGE_SIZE)));
	}
    return 0;
}

int copy_pages_app(unsigned long** start, unsigned long** end, unsigned long** dest) {
    for (long pages = 0; pages < (PAGE_ALIGN_UP((unsigned long)end - (unsigned long)start) / PAGE_SIZE); pages++) {
		UK_ASSERT(!copy_page_app(start + ADVANCE_CAPABILITY(pages * PAGE_SIZE), dest + ADVANCE_CAPABILITY(pages * PAGE_SIZE)));
	}

    return 0;
}

int set_prot_pages(unsigned long** start, unsigned long** end, unsigned long new_prot) {
    uk_pages_set_prot(start, (PAGE_ALIGN_UP((unsigned long)end - (unsigned long)start) / PAGE_SIZE), new_prot);
    return 0;
}

int set_prot_pages_nocopa(unsigned long** start, unsigned long** end, unsigned long new_prot) {
    uk_pages_set_prot_nocopa(start, (PAGE_ALIGN_UP((unsigned long)end - (unsigned long)start) / PAGE_SIZE), new_prot);
    return 0;
}

void sfork_exit(int retcode) {
    
    int pid = PROCESS_ID();

    processors[ukplat_lcpu_id()].run = 0;
    processes[pid].state = 0; //process stopped
    processes[pid].retcode = retcode;
    
#ifdef CONFIG_LIBSASSYFORK_SMP

    processors[ukplat_lcpu_id()].run = 0;
    wait_for_inst();
#else
    uk_sched_thread_exit();
#endif
}

int sfork_wait3(int *wstatus, int options, struct rusage *rusage) {
    int pid = PROCESS_ID();
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].ppid == pid && processes[i].state == 0) {
            *wstatus = processes[i].retcode;
#ifdef CONFIG_LIBSASSYFORK_SMP
	        struct uk_thread* thread = processors[processes[i].processor].thread;
	        uk_free(flexos_shared_alloc, thread->reent);
	        uk_free(flexos_shared_alloc, thread->signals_container);
	        uk_free(uk_sched_get_default()->allocator, thread);
#endif
            remove_process(i);
            return i;
        }
    }
    return 0;
}

int sfork_trampoline(void *arg) {
#ifdef CONFIG_LIBSASSYFORK_SMP
    ukarch_tlb_flush();
#endif 
    
    
    struct sfork_arg *sarg = arg;
    void *ret_cap1 = sarg->func;
    int pid = sarg->pid;
    int ppid = sarg->ppid;
    unsigned long sp_addr = offset_address_process(sarg->sp, sarg->ppid, sarg->pid);
    void* sp = __builtin_cheri_address_set(SP(), sp_addr);
    UK_ASSERT(__builtin_cheri_tag_get(sarg->sp));
    ret_cap1 = sarg->func;
    void *tls = sarg->tls;
    processes[pid].state = 1;

    asm( 
        "mov csp, %2\n"
        "mov x18, %1\n"
        "msr cid_el0, c18\n"
        "mov c18, %3\n"
        "msr ctpidr_el0, c18\n"
        "isb\n"
        "mov x0, #0\n"
        "mov clr, %0\n"
        "br %0\n"
        :
        : "r"(ret_cap1), "r"(pid), "r"(sp), "r"(tls)
        : "c18", "clr", "x0");
    return pid;
}

/*
*
*   Only a fool would attempt something this complicated without providing any documentation
*   Since I am only a partial fool, I will provide some
*   The layout of the system in memory can be simplified to the following:
*
*   ------------ <- Start (ie. address 0 if you will)
*       Kernel 
*       Things
*   ------------ <- End of kernel and beginning of application
*       Application
*       Things
*   ------------ <- End of memory
*
*   Now this is clearly an over simplification, but for our purposes it works.
*   Anything which is kernel, we map to the child and do not copy
*   Anything which is application, we either copy or we map as copy on write
*
*/

int sfork_jump_to(void* sp) {
    unsigned long ret_ptr;
    __asm__ __volatile__("mov %0, lr\n": "=r" (ret_ptr));
    return sfork_syscalls.sfork(sp, ret_ptr);
}

int sfork(void* sp, unsigned long lr) {
    unsigned long ret_ptr = lr;
    unsigned long *ret_cap;
    	
    unsigned long ppid = PROCESS_ID();
    unsigned long cpid = new_process();
    unsigned long start_copying = start_points[ppid];
    unsigned long stop_copying = end_points[ppid];
    unsigned long copy_dest = start_points[cpid];
    unsigned long offset = offset_address_process(ret_ptr, ppid, cpid);
    ret_ptr = offset_address_process(ret_ptr, ppid, cpid);
    int (*root_cap)(int, void*) = processes[cpid].cap;
    root_cap = __builtin_cheri_address_set(root_cap, (unsigned long)offset_address_process(got_start, 0, cpid));
    root_cap = __builtin_cheri_bounds_set(root_cap, end_points[0] - (unsigned long)got_start);
    root_cap = __builtin_cheri_offset_set(root_cap, ret_ptr - __builtin_cheri_base_get(root_cap));

    ret_cap = root_cap;
    UK_ASSERT(__builtin_cheri_tag_get(ret_cap)   == 1+0);

    void *newsp = int_to_cap(offset_address_process((unsigned long)sp, ppid, cpid));

    UK_ASSERT(__builtin_cheri_sealed_get((page_refs)) ==0);

    sargs[cpid].func = ret_cap;
    sargs[cpid].pid = cpid;
    sargs[cpid].ppid = ppid;
    sargs[cpid].sp = sp;
    sargs[cpid].tls = int_to_cap(offset_address_process(PROCESS_LOCAL_STORAGE(), ppid, cpid));

    
    uk_map_region(copy_dest, start_copying, (application_start_points[ppid]-start_copying)/PAGE_SIZE, PAGE_ATTR_PROT_WRITE, 0);
    uk_map_region(offset_address_process(got_start, ppid, cpid), offset_address_process(got_start, ppid, cpid), ((unsigned long)_end_got-(unsigned long)got_start)/PAGE_SIZE, PAGE_ATTR_PROT_WRITE, 0);
    copy_pages_app(int_to_cap((unsigned long)got_start), int_to_cap((unsigned long)_end_got), int_to_cap(offset_address_process(got_start, ppid, cpid)));
#ifdef CONFIG_LIBSASSYFORK_SYNC
    copy_pages(int_to_cap(application_start_points[ppid]), int_to_cap(stop_copying), int_to_cap(application_start_points[cpid]));
#endif

    /* Add copy kernel resources here */
    proc_allocators[cpid] = int_to_cap(offset_address_process(proc_allocators[ppid], ppid, cpid));
    copy_page(int_to_cap(PAGE_ALIGN_DOWN( (unsigned long)proc_allocators[ppid])), int_to_cap(PAGE_ALIGN_DOWN( (unsigned long)proc_allocators[cpid])));

    fdtable_duplicate(cpid, ppid);
    vfscore_sfork_copy_task(cpid, ppid);

#ifdef CONFIG_LIBSASSYFORK_COW
    /* Map the child to the parent starting at the application */
    uk_copy_ptes(application_start_points[ppid], end_points[ppid], application_start_points[cpid]);
    setup_page_references(cpid, ppid);
    set_prot_pages(int_to_cap(application_start_points[cpid]), int_to_cap(end_points[cpid]), PAGE_ATTR_PROT_READ);
    set_prot_pages_nocopa(int_to_cap(application_start_points[ppid]), int_to_cap(end_points[ppid]), PAGE_ATTR_PROT_READ);
    
#endif

#ifdef CONFIG_LIBSASSYFORK_COA
    uk_copy_ptes(application_start_points[ppid], end_points[ppid], application_start_points[cpid]);
    setup_page_references(cpid, ppid);
    set_prot_pages(int_to_cap(application_start_points[cpid]), int_to_cap(end_points[cpid]), PAGE_ATTR_PROT_NONE);
    set_prot_pages(int_to_cap(application_start_points[ppid]), int_to_cap(end_points[ppid]), PAGE_ATTR_PROT_NONE);
#endif

#ifdef CONFIG_LIBSASSYFORK_SMP
    
    struct uk_thread * new_thread = uk_sched_thread_create_process_no_add(uk_sched_get_default(), "Proc", NULL, &sfork_trampoline, &sargs[cpid], cpid, ppid, newsp);
    int free_processor = sfork_free_processor();
    processors[free_processor].thread = new_thread;
   
    processors[free_processor].run = 1;
    ukarch_tlb_flush();
#ifdef CONFIG_SFORK_COW
    asm("sev\n");
#endif
    processes[cpid].processor = free_processor;
#else
    uk_sched_thread_create_process(uk_sched_get_default(), "Proc", NULL, &sfork_trampoline, &sargs[cpid], cpid, ppid, newsp);
#endif
    return cpid; // the return value should be the pid of the child
}
