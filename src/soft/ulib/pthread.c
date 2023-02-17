/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-02-17
  | / /(     )/ _ \     \copyright  2021-2023 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     ulib/pthread.c
  \author   Franck Wajsburt
  \brief    Implementation on pthread API

\*------------------------------------------------------------------------------------------------*/


#include <libc.h>
#include <pthread.h>


//--------------------------------------------------------------------------------------------------
// Thread API
//--------------------------------------------------------------------------------------------------


/**
 * \brief pthread_start is the function called by the kernel at the true start of a thread
 * \param fun   is the thread's function (the one defined in the user application)
 * \param arg   is the thread argument, which will be given to the function fun
 */
static void thread_start (void *(*fun) (void *), void *arg)
{
    void * retval = fun (arg); // call the function with its arg (maybe, it will call thread_exit())
    pthread_exit(retval);      // otherwise, if fun ends, ask the kernel to exit the thread
}

int pthread_create (pthread_t * thread, pthread_attr_t * attr, void *(*fun) (void *), void *arg)
{
    return syscall_fct ((int)thread, (int)fun, (int)arg, (int)thread_start, SYSCALL_THREAD_CREATE);
}

int pthread_yield (void)
{
    return syscall_fct (0, 0, 0, 0, SYSCALL_THREAD_YIELD);
}

void pthread_exit (void *retval)
{
    syscall_fct ((int)retval, 0, 0, 0, SYSCALL_THREAD_EXIT);
}

void sched_dump (void)
{
    syscall_fct (0, 0, 0, 0, SYSCALL_SCHED_DUMP);
}

void pthread_join (pthread_t thread, void **retval)
{
    syscall_fct ((int)thread, (int)retval, 0, 0, SYSCALL_THREAD_JOIN);
}


//--------------------------------------------------------------------------------------------------
// Mutex API
//--------------------------------------------------------------------------------------------------


int pthread_mutex_init (pthread_mutex_t * mutex, pthread_mutexattr_t * attr)
{
    return syscall_fct ((int)mutex, 0, 0, 0, SYSCALL_MUTEX_INIT);
}

int pthread_mutex_destroy (pthread_mutex_t * mutex)
{
    return syscall_fct ((int)mutex, 0, 0, 0, SYSCALL_MUTEX_DESTROY);
}

int pthread_mutex_lock (pthread_mutex_t * mutex)
{
    return syscall_fct ((int)mutex, 0, 0, 0, SYSCALL_MUTEX_LOCK);
}

int pthread_mutex_unlock (pthread_mutex_t * mutex)
{
    return syscall_fct ((int)mutex, 0, 0, 0, SYSCALL_MUTEX_UNLOCK);
}


//--------------------------------------------------------------------------------------------------
// Barrier API
//--------------------------------------------------------------------------------------------------


int pthread_barrier_init (pthread_barrier_t * barrier, pthread_barrierattr_t * attr, size_t count)
{
    return syscall_fct ((int)barrier, (int) count, (int) attr, 0, SYSCALL_BARRIER_INIT);
}

int pthread_barrier_destroy (pthread_barrier_t * barrier)
{
    return syscall_fct ((int)barrier, 0, 0, 0, SYSCALL_BARRIER_DESTROY);
}

int pthread_barrier_wait (pthread_barrier_t * barrier)
{
    return syscall_fct ((int)barrier, 0, 0, 0, SYSCALL_BARRIER_WAIT);
}
