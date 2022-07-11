/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-04
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     ulib/thread.c
  \author   Franck Wajsburt
  \brief    Implementation on thread API

\*------------------------------------------------------------------------------------------------*/


#include <libc.h>
#include <thread.h>


//--------------------------------------------------------------------------------------------------
// Thread API
//--------------------------------------------------------------------------------------------------


/**
 * \brief thread_start is the function called by the kernel at the true start of a thread
 * \param fun   is the thread's function (the one defined in the user application)
 * \param arg   is the thread argument, which will be given to the function fun
 */
static void thread_start (void *(*fun) (void *), void *arg)
{
    void * retval = fun (arg); // call the function with its arg (maybe, it will call thread_exit())
    thread_exit(retval);       // otherwise, if fun ends, ask the kernel to exit the thread
}

int thread_create (thread_t * thread, void *(*fun) (void *), void *arg)
{
    return syscall_fct ((int)thread, (int)fun, (int)arg, (int)thread_start, SYSCALL_THREAD_CREATE);
}

int thread_yield (void)
{
    return syscall_fct (0, 0, 0, 0, SYSCALL_THREAD_YIELD);
}

void thread_exit (void *retval)
{
    syscall_fct ((int)retval, 0, 0, 0, SYSCALL_THREAD_EXIT);
}

void sched_dump (void)
{
    syscall_fct (0, 0, 0, 0, SYSCALL_SCHED_DUMP);
}

void thread_join (thread_t thread, void **retval)
{
    syscall_fct ((int)thread, (int)retval, 0, 0, SYSCALL_THREAD_JOIN);
}


//--------------------------------------------------------------------------------------------------
// Mutex API
//--------------------------------------------------------------------------------------------------


int thread_mutex_init (thread_mutex_t * mutex)
{
    return syscall_fct ((int)mutex, 0, 0, 0, SYSCALL_MUTEX_INIT);
}

int thread_mutex_destroy (thread_mutex_t * mutex)
{
    return syscall_fct ((int)mutex, 0, 0, 0, SYSCALL_MUTEX_DESTROY);
}

int thread_mutex_lock (thread_mutex_t * mutex)
{
    return syscall_fct ((int)mutex, 0, 0, 0, SYSCALL_MUTEX_LOCK);
}

int thread_mutex_unlock (thread_mutex_t * mutex)
{
    return syscall_fct ((int)mutex, 0, 0, 0, SYSCALL_MUTEX_UNLOCK);
}


//--------------------------------------------------------------------------------------------------
// Barrier API
//--------------------------------------------------------------------------------------------------


int thread_barrier_init (thread_barrier_t * barrier, size_t count)
{
    return syscall_fct ((int)barrier, (int) count, 0, 0, SYSCALL_BARRIER_INIT);
}

int thread_barrier_destroy (thread_barrier_t * barrier)
{
    return syscall_fct ((int)barrier, 0, 0, 0, SYSCALL_BARRIER_DESTROY);
}

int thread_barrier_wait (thread_barrier_t * barrier)
{
    return syscall_fct ((int)barrier, 0, 0, 0, SYSCALL_BARRIER_WAIT);
}
