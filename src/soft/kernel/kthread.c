/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-03
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     kernel/kthread.c
  \author   Franck Wajsburt
  \brief    scheduler & thread functions

\*------------------------------------------------------------------------------------------------*/


#include <kernel/klibc.h>


//--------------------------------------------------------------------------------------------------
// Type declarations with mandatory external accessor functions + scheduler variables
// When the declarations are static, that is because, they are only used in this file
//--------------------------------------------------------------------------------------------------

/**
 * \brief   thread_s structure which contains all we need to define a thread
 *          the size of thread_s struct is a multiple of a page and have to be aligned (on a page)
 *          thread_t structure is a pointer to a thread_t structure
 *          Be th as pointer to a struct thread_s : thread_t th
 *          th is at the beginning of a page, (that is (th & 0xFFF)==0)
 *          the top of kernel stack is (char *)th + n * PAGE_SIZE
 *          th is allocated with kmalloc(PAGE_SIZE) is thread_create_kernel()
 *          kstack_b is a pointer to the top of kernel stack, this field has to be the first
 *          field of the structure because, thus is is easy to get it
 */
struct thread_s {
    int kstack_b;                 ///< kernel stack beginning (the highest addr, outside the stack)
    int ustack_b;                 ///< user stack beginning (the highest address, outside the stack)
    int ustack_e;                 ///< user stack end (thus the lowest addr)
    list_t wait;                  ///< list element to chain threads waiting for the same resource
    spinlock_t lock;              ///< lock to protected structure during modification
    int state;                    ///< thread state from the scheduler point of view
    int *errno_a;                 ///< errno address for this thread
    void *retval;                 ///< return value
    thread_t join;                ///< expected thread in case of thread_join()
    int start;                    ///< pointer to the function which calls fun(arg)
    int fun;                      ///< pointer to the thread function (cast to int)
    int arg;                      ///< thread argument (cast to int)
    int tid;                      ///< thread identifer MUST BE PLACED JUST BEFORE CONTEXT (trace)
    int context[TH_CONTEXT_SIZE]; ///< table to store registers when thread lose the cpu
    int kstack[1];                ///< lowest address of kernel stack of thread (with MAGIC_STACK)
};

static thread_t ThreadTab[THREAD_MAX];  // simple table for the all the existing threads
static int      ThreadCurrentIdx;       // index of the current running thread
thread_t        ThreadCurrent;          // pointer to the current thread

void thread_addlast (list_t * root, thread_t thread)
{
    list_addlast (root, &thread->wait);
}

thread_t thread_item (list_t * item)
{
    return list_item (item, struct thread_s, wait);
}

int * __errno_location (void)
{
    return ThreadCurrent->errno_a;
}


//--------------------------------------------------------------------------------------------------
// scheduler of threads, all functions are static because only used by thread functions defined here
// however, sched_dump () is an external function since it is a debugging function
//--------------------------------------------------------------------------------------------------


/**
 * \brief   Insert a new thread, in the scheduler
 *          The scheduler is a simple table of all the threads
 *          To insert a new thread, we need to find a place.
 * \param   thread_new is the thread to insert
 * \return  nothing
 */
static void sched_insert (thread_t thread_new)
{
    int tid = 0;
    while ((tid < THREAD_MAX) && (ThreadTab[tid])) tid++;   // look for an empty place
    if (tid == THREAD_MAX) {                                // if not found -> exit(1);
        kprintf ("[%d] to many thread created (thread.h/THREAD_MAX\n)", clock());
        exit(1);
    }
    thread_new->tid = tid;                                  // set thread identifier
    ThreadTab[tid] = thread_new;                            // store the new thread
    if (ThreadCurrent == NULL)                              // first thread insertion
        ThreadCurrent = thread_new;
}

/**
 * \brief   Gives the next ThreadCurrentIdx to execute
 *          There are two loops.
 *          at the beginning, sched_elect() goes through the scheduler's thread table ONCE
 *          to find a READY thread. On success, it returns the new ThreadCurrentIdx
 *          In the second loop, if no READY thread has been found, then IRQs are enabled,
 *          and sched_elect() searches until it finds a READY thread
 *          Thus, if thread_yield() is called by timer_isr(), the ThreadCurrent is necessarly
 *          READY and then, sched_elect() will never enable the IRQ (it is forbidden to do so).
 *          __attribute__((noinline)) is to see this function is trace debug
 * \return  the next thread number to execute, it could be unchanged if there is not any else
 */
static __attribute__((noinline)) int sched_elect (void)
{
    // ---- First search for a READY state with IRQ disabled

    int th, thmax;
    th = thmax = (ThreadCurrentIdx + 1) % THREAD_MAX;       // start with the current thread + 1
    do {
        if ( ThreadTab[th] &&
            (ThreadTab[th]->state == TH_STATE_READY)) {     // thread READY found
            return th;                                      // return the chosen one
        }
        th = (th+1) % THREAD_MAX;                           // go to the next (circular course)
    } while (th != thmax);                                  // Only one loop

    // ---- Then if there is no READY thread then wait for it with IRQ enabled

    irq_enable();
    while ( (ThreadTab[th] == NULL) || (ThreadTab[th]->state != TH_STATE_READY)) {
        th = (th+1) % THREAD_MAX;                           // we search as long as we do not found
    }
    irq_disable();
    return th;                                              // return the chosen one
}

/**
 * \brief   Change the current thread for another, it can be unchanged if it is alone
 */
static void sched_switch (void)
{
    int th_next = sched_elect ();                           // get a next ready thread
    if (th_next != ThreadCurrentIdx) {                      // if it is not the same
        if (thread_context_save (ThreadCurrent->context)) { // Save current context, and return 1
            ThreadCurrentIdx = th_next;                     // update ThreadCurrentIdx
            ThreadCurrent = ThreadTab[th_next];             // update ThreadCurrent
            thread_context_load (ThreadCurrent->context);   // load contxt, exit thread_context_save
        }                                                   // but with 0 as return value
    }
    ThreadCurrent->state= TH_STATE_RUNNING;                 // the chosen one is RUNNNIG
}

/**
 * \brief   Dump all the threads that are in the scheduler
 */
#define Y EC_YELLOW
#define W EC_WHITE
#define G EC_GREEN
#define M EC_MAGENTA
#define O EC_ORANGE
#define S M "%s" W
#define P G "%p" W
#define D O "%d" W
void sched_dump (void)
{
    char *state_name[16] = {
        [TH_STATE_RUNNING] = "RUNNING",
        [TH_STATE_READY] = "READY",
        [TH_STATE_DEAD] = "DEAD",
        [TH_STATE_WAIT] = "WAIT",
        [TH_STATE_ZOMBIE] = "ZOMBIE",
    };

    kprintf (Y"-------------------------- DUMP ALL THREADS ---------------------------\n");
    kprintf (W"\007thread current ("P") : "D"\n", ThreadCurrent, ThreadCurrentIdx);
    for (int th = 0; th < THREAD_MAX; th++) {
        thread_t thread = ThreadTab[th];
        if (thread) {
           kprintf (Y"----------------------------------------------------------------------- ");
           kprintf (D"\n", thread->tid);
           kprintf ("["D"] thread: "P,  clock (), thread);
           kprintf ("   errmsg: "S"\n", errno_mess[*thread->errno_a+1]);
           kprintf (" - state:     "S"\t", state_name[thread->state]);
           kprintf ("   wait.next: "P"\t", thread->wait.next);
           kprintf ("   wait.prev: "P"\n", thread->wait.prev);
           kprintf (" - retval:    "P"\t", thread->retval);
           kprintf ("   join:      "P"\t", thread->join);
           kprintf ("   errno:     "P"\n", *thread->errno_a);
           kprintf (" - start:     "P"\t", thread->start);
           kprintf ("   function:  "P"\t", thread->fun);
           kprintf ("   arg:       "P"\n", thread->arg);
           kprintf (" - ustack_b:  "P" ("P")\t", thread->ustack_b, *(int*)thread->ustack_b);
           kprintf (  " ustack_e:  "P" ("P")\n", thread->ustack_e, *(int*)thread->ustack_e);
           kprintf (" - kstack_b:  "P" ("P")\t", thread->kstack_b, *(int*)thread->kstack_b);
           kprintf (  " kstack_e:  "P" ("P")\n", &thread->kstack, thread->kstack[0]);
/*
           kprintf (" - context:");
           kprintf ("   S0: "P,          thread->context[TH_CONTEXT_S0]);
           kprintf ("   S1: "P,          thread->context[TH_CONTEXT_S1]);
           kprintf ("   S2: "P,          thread->context[TH_CONTEXT_S2]);
           kprintf ("   S3: "P"\n\t   ", thread->context[TH_CONTEXT_S3]);
           kprintf ("   S4: "P,          thread->context[TH_CONTEXT_S4]);
           kprintf ("   S5: "P,          thread->context[TH_CONTEXT_S5]);
           kprintf ("   S6: "P,          thread->context[TH_CONTEXT_S6]);
           kprintf ("   S7: "P"\n\t   ", thread->context[TH_CONTEXT_S7]);
           kprintf ("   S8: "P,          thread->context[TH_CONTEXT_S8]);
           kprintf ("   SR: "P,          thread->context[TH_CONTEXT_SR]);
           kprintf ("   RA: "P,          thread->context[TH_CONTEXT_RA]);
           kprintf ("   SP: "P"\n",      thread->context[TH_CONTEXT_SP]);
*/
        }
    }
    kprintf (Y"------------------------ END DUMP ALL THREADS -------------------------\n");
}


//--------------------------------------------------------------------------------------------------
// Thread functions
//--------------------------------------------------------------------------------------------------


/**
 * \brief   thread_bootstrap() function is the bootstrap of the thread.
 *          It means that it is the very first function we call when the thread_context_load()
 *          returns for the very first time. In fact, thread_context_load() ends with a "jr $31" 
 *          as usual but when we create a thread, $31 is initialized with the thread_bootstrap()
 *          address. Thus, when we return from thread_context_load() the very first time, we enter
 *          in thread_bootstrap(). The function thread_bootstrap() cannot have any argument,
 *          since thread_context_load() does not restore $4 to $7 registers (which are the 
 *          registers of arguments). So, that is the thread_bootstrap() itself that will call the
 *          threat_start() function with the needed arguments, as we know what is the current
 *          thread, and we are able to find out what is the user function to call with which args.
 *          As the thread is just beginning to run, its state is now RUNNING
 *
 *          To sum up what is happening when a thread context is loaded:
 *
 *          - the very first time that a thread is chosen by the scheduler
 *              thread_context_load (thread)                                   (def hcpua.S)
 *              - thread_bootstrap ()                                          (def kthread.c)
 *                - thread_launch (thread->fun, thread->arg, thread->start)    (def hcpua.S)
 *                  - thread->thread_start (thread->fun, thread->arg)          (def thread.c)
 *                    - thread->fun (thread->arg)                              (def uapp)
 *
 *          - the others times that a thread is chosen
 *              thread_context_load (thread)                                   (def hcpua.S)
 *              - return to sched_switch()                                     (def kthread.c)
 *                - return to the caller of sched_switch, 2 possibilities:
 *                  1. from a syscall
 *                     - return to the syscall function (e.g. tty_read)        (def harch.c)
 *                       - return to syscall_handler                           (def hcpua.S)
 *                         - return to the app that uses the syscall           (def app)
 *                  2. from a isr (here, only isr_timer)
 *                     - return to isrcall                                     (def harch.c)
 *                       - return to irq_handler                               (def hcpua.S)
 *                         - return to the interrupted code                    (def kernel OR uapp)
 */
static void thread_bootstrap (void)
{
    thread_t thread = ThreadCurrent;                            // gets the current thread
    thread->state = TH_STATE_RUNNING;                           // the thread is now RUNNING
    thread_launch (thread->fun, thread->arg, thread->start);    // calls : start(fun,arg)
}

int thread_create (thread_t * thread_p, int fun, int arg, int start)
{
    thread_t thread = kmalloc (PAGE_SIZE);                      // thread is thus always aligned
    if (thread == NULL) return EAGAIN;                          // Not enough memory
    thread->kstack_b = (int)thread + PAGE_SIZE - 4;             // kstack beginning (highest addr)
    thread->ustack_b = (int)malloc_ustack();                    // stack beginning (highest address)
    thread->ustack_e = thread->ustack_b - USTACK_SIZE + 4;      // stack end (lowest addr)
    thread->state    = TH_STATE_READY;                          // it can be chosen by the scheduler
    list_init (&thread->wait);                                  // initialize the waiting list
    thread->retval   = NULL;                                    // default return value
    thread->join     = NULL;                                    // no awaited thread
    thread->start    = start;                                   // start() will call fun(arg)
    thread->fun      = fun;                                     // function of the thread
    thread->arg      = arg;                                     // argument of the thread
    thread->errno_a  = (int*)(thread->ustack_b - 4);            // errno is the first int of ustack
    thread_context_init(thread->context,                        // table to store context
                        thread_bootstrap,                       // thread_bootstrap() to begin
                        thread->errno_a);                       // stack pointer addr (here errno)
/*
    thread->context[TH_CONTEXT_SR] = 0x413;                     // UM=1 EXL=1 IE=1
    thread->context[TH_CONTEXT_RA] = (int)thread_bootstrap;     // goto thread_bootstrap
    thread->context[TH_CONTEXT_SP] = (int)thread->errno_a;      // stack beginning at errno address
*/
    *(int*)thread->kstack_b = MAGIC_STACK;                      // should not be erased
    thread->kstack[0] = MAGIC_STACK;                            // should not be erased

    sched_insert (thread);                                      // insert new thread in scheduler
    *thread_p = thread;
    errno = SUCCESS;
    return SUCCESS;
}

void thread_main_load (thread_t thread)
{
    thread_context_load (thread->context);
}

/**
 * Use this function to yield the CPU while the thread is in a RUNNING state, which means
 * that the thread is not waiting for any resource (as a device, mutex or anything else),
 * so just we need to put it in a READY state and try to switch to another thread.
 * It is not necessary to take the thread lock, since no one will change the state at this moment
 * If thread is the only READY, it will take the CPU again.
 */
int thread_yield (void)
{
    ThreadCurrent->state = TH_STATE_READY;                      // yield the CPU but always READY
    sched_switch ();                                            // Try to change thread
    return SUCCESS;
}

/**
 * thread_exit() is to exit the current thread. Read this comment then the thread_join's comment.
 * E1) firstly, store the return value and tell the current thread is becoming ZOMBIE
 * E2) If the task waiting for the end of the current thread is already known
 * E3) -- then since, this task is waiting, change it state to READY
 * Finally, definitively yield the processor (the current thread never come back
 * Critical section:
 *   In case of real parallelism with thread_join, we must avoid the sequence: J1 J2 E1 E2 E3 J3
 *   because then, the thread that executes the join gets the state WAIT and won't be never notified
 *   Thus, a critical section is created using the thread structure lock of the expected thread
 */
void thread_exit (void *retval)
{
    INFO ("thread %p retval %p\n", ThreadCurrent, retval);

    ThreadCurrent->retval = retval;                             // E1: store the return value
    ThreadCurrent->state = TH_STATE_ZOMBIE;                     //     tell that is the end

    spin_lock (&ThreadCurrent->lock);                           // avoid sequence J1 J2 E1 E2 E3 J3
    if (ThreadCurrent->join != NULL)                            // E2: if there is a thread waiting
        ThreadCurrent->join->state = TH_STATE_READY;            // E3: then change its state
    spin_unlock (&ThreadCurrent->lock);                         // end of critical section
    sched_switch ();                                            // at last, definitively yield proc
}

/**
 * Please, read first the thread_exit() comment. thread_join() is to wait another thread.
 * J1) firstly, tell the excepted thread that the current thread is waiting for it
 * J2) If the expected thread is still alive,
 * J3) -- then wait for it
 * if the expected thread is already a ZOMBIE, so take its return value and change it to DEAD
 * Critical section: see the comment of thread_exit
 * TODO check is thread_expected is a real thread (add a MAGIC number is struct thread_s
 */
int thread_join (thread_t thread_expected, void **retval)
{
    INFO ("thread %p expects %p\n", ThreadCurrent, thread_expected);

    if (thread_expected == NULL) return ESRCH;                  // Error code
    thread_expected->join = ThreadCurrent;                      // J1: tell ThreadCurrent is waiting
    spin_lock (&thread_expected->lock);                         // avoid J1 J2 E1 E2 E3 J3
    if (thread_expected->state != TH_STATE_ZOMBIE) {            // J2: if expected thread not ended
        ThreadCurrent->state = TH_STATE_WAIT;                   // J3: then wait for it
        spin_unlock (&thread_expected->lock);                   // end of critical section
        sched_switch ();                                        // yield the proc till READY
    } else {
        spin_unlock (&thread_expected->lock);                   // end of critical section
    }
    *retval = thread_expected->retval;                          // get the return value
    thread_expected->state = TH_STATE_DEAD;                     // finally, the thread is DEAD

    return SUCCESS;
}

/**
 * thread_wait() and thread_notify() are made to handle the WAIT state, please read both comments.
 * Use this function to put the current thread (say T0) into a WAIT state. We need this when T0
 * has just tried to acquire a busy resource. We assume that T0 has already been added to the
 * resource's waiting list, so we still need to detach T0 from its processor's availability list.
 * The problem is that the resource may become available during the state change of T0.
 * Suppose another thread (say T1) returns the resource and calls thread_notify(T0).
 * T1 is necessarily running on another CPU, so there is a risk that T0 will become READY
 * (because of the call to thread_notify(T0)) with the resource in its possession and then
 * become WAIT (because of the call to thread_wait() just after).
 * In this case, T0 will never notified again. This is the end.
 * there are two possibilities: A [ wait then notify ] or B [ notify then wait ]
 * A) That is the normal case. T0 is first detached from ready list,
 *    then secondly, T0 is notified and become READY
 * B) That is a abnormal case. T0 begins to notify, because T1 gives it the resource and
 *    calls thread_notify() then T0 becomes READY, but just after that T0 calls thread_wait()!
 *    Thus, thread_wait(), called by T0, must know that thread_notify() has already been called,
 *    it is simply possible by looking its state. If T0 is always RUNNING, then it means that
 *    thread_notify() has not been called yet, because otherwise the state would have been READY
 *    In that case T0 has to wait, otherwise T0 is READY and we DO NOT change the state
 *    The lock is to protect the sequence [ read - test - modification ]
 * Note that it is possible that the notify() occurs after unlocking and just before sched_switch().
 * In this case, T0 will be READY but will lose the CPU even if it owns the resource.
 * This is unfortunate, as the resource is locked for a quantum of time, but it is not fatal.
 */
void thread_wait (void)
{
    spin_lock (&ThreadCurrent->lock);                           // !--! critical section
    if (ThreadCurrent->state == TH_STATE_RUNNING)               // thread_notify not happened yet
         ThreadCurrent->state = TH_STATE_WAIT;                  // give the CPU back
    spin_unlock (&ThreadCurrent->lock);                         // !--! end of critical section
    sched_switch ();                                            // then switch the thread
}

/**
 * Start to read the thread_wait() comment to understand what is T0 and T1.
 * T1 calls the tread_notify() function when the resource expected by T0 has occurred.
 * When T1 enter the critical section of thread_notify, there are two possibilities
 * 1) T0 already passed through the critical section of thread_wait and so T0 is in WAIT
 * 2) T0 has not yet crossed the critical section of thread_wait and T0 is therefore in RUNNING yet.
 * In both cases, we must ask T0 to notify, which is done by setting T0's state to READY
 */
void thread_notify (thread_t thread)
{
    spin_lock (&ThreadCurrent->lock);                           // !--! critical section
    thread->state = TH_STATE_READY;                             // (RUNNING or WAIT) to READY
    spin_unlock (&ThreadCurrent->lock);                         // !--! end of critical section
}
