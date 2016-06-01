// -----------------------------------------------------------------------------
// Multi-Processing Module
// -----------------------
// 
// General  :   The module manages multi-processing and multi-threading.
//
// Input    :   None
//
// Process  :   Manages multi-processing and multi-threading.
//
// Output   :   None
//
// -----------------------------------------------------------------------------
// Programmer   :   Eden Frenkel
// -----------------------------------------------------------------------------

#include <processing.h>


const char *STATUSES[3] = {"Dead", "Idle", "Active"};

static uint32_t next_pid;
static ThreadNode *current_node;
static unsigned int status_ticks[] = {0, 4, 64};

// -----------------------------------------------------------------------------
// init_processing
// ---------------
// 
// General      :   The function initializes the scheduling mechanism.
//
// Parameters   :   None
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void init_processing(void) {
    ProcessNode *kernel;
    ThreadNode *main_thread;

    kernel = (ProcessNode *) malloc(sizeof (ProcessNode));
    main_thread = (ThreadNode *) malloc(sizeof (ThreadNode));

    kernel->name = "kernel";
    kernel->source = "~~KERNEL~BASE~~";
    kernel->pid = 0;
    kernel->ppid = 0;
    asm("movl %%cr3, %%eax" : "=a" (kernel->cr3));
    kernel->pages = 0;
    kernel->status = ACTIVE;
    kernel->next_tid = 1;
    kernel->next = kernel;

    main_thread->name = "kernel_main";
    main_thread->proc = kernel;
    main_thread->status = ACTIVE;
    main_thread->stack_page = main_thread->p_stack_page = 0x20000 - PAGE_SIZE;
    main_thread->init = 1;
    main_thread->next = 0;

    current_node = kernel->main_thread = main_thread;
    next_pid = 1;

    init_switch(status_ticks[current_node->status]);
}

// -----------------------------------------------------------------------------
// switch_proc
// -----------
// 
// General      :   The function switchs the running task.
//
// Parameters   :   None
//
// Return Value :   The amount of ticks to run it
//
// -----------------------------------------------------------------------------

unsigned int switch_proc(void) {
    char *dest;
    char *src;
    uint32_t n;
    ThreadNode *next_node;
    void *dead;
    PDE *pde;
    PTE *pte;

    asm("pushal");
    asm volatile ("movl %%esp, %%eax" : "=a" (current_node->stack_pointer));

    while (current_node->next && current_node->next->status == DEAD) {
        dead = (void *) current_node->next;
        free((void *) current_node->next->name);
        current_node->next = current_node->next->next;
        free(dead);
    }
    while (!current_node->next && current_node->proc->next->main_thread->status == DEAD) {
        if (current_node->proc->next->main_thread->next) {
            dead = (void *) current_node->proc->next->main_thread;
            current_node->proc->next->main_thread = current_node->proc->next->main_thread->next;
            free(dead);
        } else {
            dead = (void *) current_node->proc->next;
            current_node->proc->next = current_node->proc->next->next;
            free(dead);
        }
    }

    if (current_node->next)
        next_node = current_node->next;
    else
        next_node = current_node->proc->next->main_thread;

    if (!next_node->init) {
        dest = (char *) next_node->p_stack_page;
        src = (char *) current_node->stack_page;
        n = PAGE_SIZE;

        while (n--)
            *dest++ = *src++;

        next_node->stack_pointer = current_node->stack_pointer;
        next_node->init = 1;
    }

    if (current_node->proc->pid == next_node->proc->pid) {

        pde = (PDE *) ((next_node->proc->cr3 & 0xFFFFF000) | ((next_node->stack_page >> 20) & 0x00000FFC));
        pte = (PTE *) (((pde->base_address << 12) & 0xFFFFF000) | ((next_node->stack_page >> 10) & 0x00000FFC));

        pte->base_address = next_node->p_stack_page >> 12;
    }
    current_node = next_node;

    asm volatile ("movl %%eax, %%cr3" : : "a" (current_node->proc->cr3));
    asm volatile ("movl %%eax, %%esp" : : "a" (current_node->stack_pointer));
    asm("popal");
    return status_ticks[current_node->status];
}

// -----------------------------------------------------------------------------
// new_thread
// ----------
// 
// General      :   The function creates a new thread of the current process and
//					adds it to the scheduling queue.
//
// Parameters   :
//              name    -   A pointer to a string representing the thread's name (In)
//              status  -   A pointer to a pointer to an unsigned int; the pointer the
//                          pointer points to will be set to point to an int that
//                          represents the thread's status (Out)
//
// Return Value :   1 if it is the thread, otherwise 0 (similar to fork)
//
// -----------------------------------------------------------------------------

int new_thread(const char *name, uint32_t **status) {
    ThreadNode *thread;

    CLEAR_INTS();

    thread = (ThreadNode *) malloc(sizeof (ThreadNode));

    thread->name = strcpy((char *) malloc(strlen(name) + 1), name);
    thread->proc = current_node->proc;
    thread->status = ACTIVE;
    thread->tid = current_node->proc->next_tid++;
    thread->p_stack_page = (uint32_t) palloc();
    thread->stack_page = current_node->stack_page;
    thread->init = 0;

    thread->next = current_node->next;
    current_node->next = thread;

    SET_INTS();

    while (!thread->init)
        HALT();
    if (thread->tid == current_node->tid) {
        return 1;
    }
    *status = &thread->status;
    return 0;
}

// -----------------------------------------------------------------------------
// dispose_thread
// --------------
// 
// General      :   The function disposed of the current thread.
//
// Parameters   :   None
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void dispose_thread(void) {
    CLEAR_INTS();
    current_node->status = DEAD;
    pfree((void *) (current_node->p_stack_page));
    SET_INTS();
    while (1)
        HALT();
}

// -----------------------------------------------------------------------------
// wait
// ----
// 
// General      :   The function waits for a process/thread to terminate.
//
// Parameters   :	
//              status  -   A pointer to an unsign int that represents the status of the
//                          process/thread (In)
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void wait(uint32_t *status) {
    current_node->status = IDLE;
    while (*status)
        HALT();
    current_node->status = ACTIVE;
}

// -----------------------------------------------------------------------------
// print_proc_data
// ---------------
// 
// General      :   The function prints information about processes running on
//                  the system.
//
// Parameters   :
//              with_pid        -   Whether to print the Process ID of each process (In)
//              with_ppid       -   Whether to print the Parent-Process ID of each process (In)
//              with_status     -   Whether to print the Status of each process (In)
//              with_threads    -   Whether to print the threads of each process (In)
//              with_tid        -   Whether to print the Thread Id of each thread (In)
//              with_tstatus    -   Whether to print the Status of each thread (In)
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void print_proc_data(int with_pid, int with_ppid, int with_status, int with_threads, int with_tid, int with_tstatus) {
    ProcessNode *p;
    ThreadNode *t;
    char *buff;

    CLEAR_INTS();
    p = current_node->proc;
    buff = malloc(16);
    do {
        puts(p->name);
        if (with_pid) {
            puts("\tPID: ");
            puts(uitoa(p->pid, buff, BASE10));
        }
        if (with_ppid) {
            puts("\tPPID: ");
            puts(uitoa(p->ppid, buff, BASE10));
        }
        if (with_status) {
            puts("\tStatus: ");
            puts(STATUSES[p->status]);
        }
        putc('\n');
        if (with_threads) {
            t = p->main_thread;
            while (t) {
                putc('\t');
                puts(t->name);
                if (with_tid) {
                    puts("\tTID: ");
                    puts(uitoa(t->tid, buff, BASE10));
                }
                if (with_tstatus) {
                    puts("\tStatus: ");
                    puts(STATUSES[t->status]);
                }
                putc('\n');
                t = t->next;
            }
        }
        p = p->next;
    } while (p != current_node->proc);
    free(buff);
}

// -----------------------------------------------------------------------------
// set_idle
// --------
// 
// General      :   The function sets the current thread's status to IDLE.
//
// Parameters   :   None
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void set_idle(void) {
    current_node->status = IDLE;
}

// -----------------------------------------------------------------------------
// set_active
// ----------
// 
// General      :   The function sets the current thread's status to ACTIVE.
//
// Parameters   :   None
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void set_active(void) {
    current_node->status = ACTIVE;
}

// -----------------------------------------------------------------------------
// kill_th
// -------
// 
// General      :   The function kills the specified thread.
//
// Parameters   :
//		pid -   The Process ID of the thread's process (In)
//		tid -   The Thread ID of the thread (In)
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void kill_th(uint32_t pid, uint32_t tid) {
    ProcessNode *p;
    ThreadNode *t;

    CLEAR_INTS();
    if (!pid && !tid) {
        SET_INTS();
        puts("Cannot kill Main Kernel thread.\n");
        return;
    }
    p = current_node->proc;
    do {
        if (p->pid == pid) {
            t = p->main_thread;
            do {
                if (t->tid == tid) {
                    t->status = DEAD;
                    pfree((void *) (current_node->p_stack_page));
                    SET_INTS();
                    puts("Thread was killed successfully.\n");
                    return;
                }
                t = t->next;
            } while (t);
        }
        p = p->next;
    } while (p != current_node->proc);
    SET_INTS();
    puts("Thread does not exists.\n");
}