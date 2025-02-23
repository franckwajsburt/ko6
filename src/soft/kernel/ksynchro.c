/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-02-23
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     kernel/ksynchro.c
  \author   Franck Wajsburt
  \brief    synchronization service function for the user application

\*------------------------------------------------------------------------------------------------*/

#include <kernel/klibc.h>


//--------------------------------------------------------------------------------------------------
// Mutex API
//--------------------------------------------------------------------------------------------------


struct thread_mutex_s {
    spinlock_t  lock;           ///< protection against parallel modifications
    unsigned    busy;           ///< 1 mutex is busy ; 0 mutex is free
    list_t      wait;           ///< list element to chain threads that are wainting for the mutex
    thread_t    owner;          ///< a thread has to be locked and unlocked by the same thread
};

/**
 * Create and initialize a mutex
 */
int thread_mutex_init (thread_mutex_t * mutex)
{
    thread_mutex_t m = kmalloc (sizeof (*m));                   // Get memory
    if (m == NULL) return ENOMEM;                               // malloc impossible
    m->lock = 0;                                                // lock to protect
    m->busy = 0;                                                // busy is free at first
    list_init (&m->wait);                                       // no waiting threads
    m->owner = NULL;                                            // no thread owns the mutex
    *mutex = m;                                                 // at last save the new mutex
    return SUCCESS;
}

/**
 * Destroy the given mutex if possible
 */
int thread_mutex_destroy (thread_mutex_t * mutex)
{
    thread_mutex_t m = *mutex;
    if (m == NULL) return EINVAL;                               // unitialized mutex
    if (m->busy) return EBUSY;                                  // try to destroy an lock mutex
    if (m->owner != ThreadCurrent) return EPERM;                // the thread does not own the mutex
    kfree (m);
    return SUCCESS;
}

/**
 * lock = Acquire the mutex.
 * First, check if the arguments are legal, then take the lock that protects the mutex,
 * it is an active loop but not too long.
 * After that, there are two possibilities: A) the mutex is busy, B) the mutex is free.
 * A) busy case: ThreadCurrent is added to the end of the mutex's pending thread list using
 *    thread_addlast(). This function is defined in the thread API (kthread.c), note that we cannot
 *    use list_addlast() as we do not have access to the thread structure (it is a hidden structure
 *    for security reasons).
 *    Next, we need to release the lock, then finally the current thread needs to change its
 *    state from RUNNING to WAIT. When the thread that currently owns the mutex releases it,
 *    then it will give ownership of the mutex by leaving the state occupied, but changing
 *    the owner field. Note that it is possible that the release of the mutex occurs just after
 *    the current thread has decided to wait, there could be a risk that it will start waiting
 *    while it owns the mutex. This case is handled by the thread_wait() and thread_notify()
 *    functions, details are in the comments of these functions in kthread.c.
 * B) free case : it's quite simple, it just have to set busy to 1 and fill the ownership field,
 *    and release the lock.
 */
int thread_mutex_lock (thread_mutex_t * mutex)
{
    thread_mutex_t m = *mutex;
    if (m == NULL) return EINVAL;                           // unitialized mutex
    if (m->busy && (m->owner == ThreadCurrent)) return EDEADLK; // try to lock several times

    spin_lock (&m->lock);                                   // take the lock of the mutex
    if (m->busy) {                                          // if the mutex is busy
        thread_addlast (&m->wait, ThreadCurrent);           // put the current thread waiting
        spin_unlock (&m->lock);                             // give the lock back
        thread_wait ();                                     // tell the thread to wait
    } else {
        m->busy = 1;                                        // take the mutex
        m->owner = ThreadCurrent;                           // set the ownership
        spin_unlock (&m->lock);                             // give the lock back
    }
    return SUCCESS;                                         // and return with success
}

/**
 * unlock : release the mutex
 * First, check if the arguments are legal, then take the lock that protects the mutex,
 * Then, get the thread in the thread waiting list of the mutex.
 * There are two possibilities: A) there is one or B) there is not.
 * A) There is a waiting thread, leave the busy field set to 1, change the property and notify
 *    the waiting thread with thread_notify() and it will become READY. If the new mutex owner
 *    decide to wait while the current thread try to notify it, there could be a risk that the
 *    new mutex owner waits definitively with the mutex ownership. This case is handled by the
 *    functions thread_wait() and thread_notify(), see details in kthread.c
 * B) There is no waiting thread, reset busy and owner.
 * At last, in both cases, the mutex lock is released.
 */
int thread_mutex_unlock (thread_mutex_t * mutex)
{
    thread_mutex_t m = *mutex;
    if (m == NULL) return EINVAL;                           // unitialized mutex
    if (m->busy == 0) return EINVAL;                        // unlocked an unlock mutex
    if (m->owner != ThreadCurrent) return EPERM;            // the thread does not own the mutex

    spin_lock (&m->lock);                                   // take the lock of the mutex
    list_t * waiting_item = list_getfirst(&m->wait);        // get thread from waiting list
    if (waiting_item) {                                     // if there is a thread waiting
        m->owner = thread_item (waiting_item);              // get the new mutex owner
        thread_notify (m->owner);                           // this thread owns the mutex
    } else {                                                // no thread is waiting
        m->busy = 0;                                        // free the mutex
        m->owner = NULL;                                    // delete the owner field
    }
    spin_unlock (&m->lock);                                 // give the lock back
    return SUCCESS;
}


//--------------------------------------------------------------------------------------------------
// Barrier API
//--------------------------------------------------------------------------------------------------


#define MAGIC_BARRIER   0xDEADBABA

struct thread_barrier_s {
    int         magic;          ///< magic number to check the validity of the barrier
    spinlock_t  lock;           ///< protection against parallel modifications
    size_t      expected;       ///< number of expected threads
    size_t      waiting;        ///< number of threads waiting
    list_t      wait;           ///< list element to chain threads that are wainting for the mutex
};

int thread_barrier_init (thread_barrier_t * barrier, size_t count)
{
    thread_barrier_t b = *barrier;                          // get the barrier pointer

    if (count == 0) return EINVAL;                          // count must be > 0

    if (b == NULL) {                                        // if we need a new barrier
        b = kmalloc (sizeof (struct thread_barrier_s));     // allocates a new barrier
        if (b == NULL) return ENOMEM;                       // test if there is enough memory
        b->magic = MAGIC_BARRIER;                           // tell it is a BARRIER
        b->lock = 0;                                        // free the lock
        b->expected = count;                                // init the expected threads
        b->waiting = 0;                                     // init the counter of waiting threads
        list_init (&b->wait);                               // init the waiting list
        *barrier = b;                                       // at last, init. the return variable
        return SUCCESS;                                     // it's fine
    }

    if (b && (b->magic != MAGIC_BARRIER)) return EINVAL;    // it is not an old barrier

    spin_lock (&b->lock);                                   // get the ownership
    if (b->waiting != 0) {                                  // if someone is waiting
        spin_lock (&b->lock);                               // release the ownership
        return EBUSY;                                       // return an error
    }

    b->expected = count;                                    // set the number of expected threads
    spin_lock (&b->lock);                                   // release the ownership

    return SUCCESS;                                         // it's fine
}

int thread_barrier_wait (thread_barrier_t * barrier)
{
    thread_barrier_t b = *barrier;                          // get the barrier pointer

    if (b == NULL) return EINVAL;                           // b is not created
    if (b && (b->magic != MAGIC_BARRIER)) return EINVAL;    // check that it is barrier (MAGIC)

    spin_lock (&b->lock);                                   // get the ownership
    b->waiting++;                                           // the current thread is the newcomer
    if (b->waiting == b->expected) {                        // if all the expected threads are there
        list_foreach (&b->wait, waiting_item) {             // then for each thread waiting
            list_unlink (waiting_item);                     // get thread from waiting list
            thread_t t = thread_item (waiting_item);        // get the pointer of a waiting thread
            thread_notify (t);                              // and notifies it
        }
        b->waiting = 0;                                     // init the counter of waiting threads
        spin_unlock (&b->lock);                             // release the ownership
    } else {
        thread_addlast (&b->wait, ThreadCurrent);           // the current thread in the wait. list
        spin_unlock (&b->lock);                             // release the lock
        thread_wait ();                                     // tell the thread to wait
    }

    return SUCCESS;                                         // it's fine
}

int thread_barrier_destroy (thread_barrier_t * barrier)
{
    thread_barrier_t b = *barrier;                          // get the barrier pointer

    if (b == NULL) return EINVAL;                           // b is not created
    if (b && (b->magic != MAGIC_BARRIER)) return EINVAL;    // check that it is barrier (MAGIC)

    spin_lock (&b->lock);                                   // get the ownership
    if (b->waiting != 0) {                                  // if barrier is in use
        spin_unlock (&b->lock);                             // release the lock
        return EBUSY;                                       // return an error
    }
    kfree (b);                                              // destroys barrier & erases its memory

    return SUCCESS;                                         // erasing the memory releases the lock
}
