/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-02-17
  | / /(     )/ _ \     \copyright  2021-2023 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     ulib/pthread.h
  \author   Franck Wajsburt
  \brief    Thread API

\*------------------------------------------------------------------------------------------------*/


#ifndef _THREAD_H_
#define _THREAD_H_


//--------------------------------------------------------------------------------------------------
// Pthread API
//--------------------------------------------------------------------------------------------------


/**
 * \brief   hidden pthread type, the user do not know what is in the thread structure
 */
typedef struct pthread_s * pthread_t;

/**
 * \brief   hidden pthread attribute type
 */
typedef struct pthread_attr_s * pthread_attr_t;

/**
 * \brief   asks the kernel via a syscall to start a new thread by invoking fun(arg)
 *          fun() can stop the thread by calling thread_exit(value)
 *          or if it returns a value, it is equivalent to call thread_exit with this value
 *          The state of the new thread is TH_STATE_READY.
 *
 * \param   thread  pointer to the thread structure
 * \param   attr    pointer to the thread atribute structure (default if NULL)
 * \param   fun     pointer to the function of the thread
 * \param   arg     the argument given to fun()
 */
extern int pthread_create (pthread_t *thread, pthread_attr_t *attr, void *(*fun)(void*), void *arg);

/**
 * \brief   Causes the current thread to give up the CPU in order to give it to another
 */
extern int pthread_yield (void);

/**
 * \brief   terminates the current thread and returns retval
 * \param   retval the value can be retrieved by calling the thread_join function of another thread
 */
extern void pthread_exit (void *retval);

/**
 * \brief   wait for a thread termination
 * \param   thread  expected thread 
 * \param   retval  pointer to the return value of the expected thread
 */
extern void pthread_join (pthread_t thread, void **retval);

/**
 * \brief   Displays on the console (tty0) all active threads, it is for debugging.
 */
void sched_dump(void);


//--------------------------------------------------------------------------------------------------
// Mutex API
//--------------------------------------------------------------------------------------------------


/**
 * \brief   hidden mutex type
 */
typedef struct pthread_mutex_s * pthread_mutex_t;

/**
 * \brief   hidden mutex attribute type
 */
typedef struct pthread_mutexattr_s * pthread_mutexattr_t;

/**
 * \brief   creates a new mutex and initializes mutex variable with the new mutex (side effect)
 *          it is a mutex with error checking 
 * \param   mutex a pointer referencing the new mutex 
 * \param   attr a pointer to the attribute of the new mutex (NULL for default) 
 * \return  0 on success, 1 on fealure
 */
extern int pthread_mutex_init (pthread_mutex_t * mutex, pthread_mutexattr_t * attr);

/**
 * \brief   lock the referenced mutex, it is a blocking operation.
 *          If the mutex is already locked, the calling thread blocks until the mutex becomes 
 *          available. If the mutex does not exist or is already locked, this is an error
 * \param   mutex a pointer referencing a mutex 
 * \return  0 on success, 1 on fealure
 */
extern int pthread_mutex_lock (pthread_mutex_t * mutex);

/**
 * \brief   unlock the referenced mutex
 *          If the mutex does not exist or is not locked or has been locked by another, 
 *          this is an error that causes exit with error
 * \param   mutex a pointer referencing a mutex 
 * \return  0 on success, 1 on fealure
 */
extern int pthread_mutex_unlock (pthread_mutex_t * mutex);

/**
 * \brief   destroy the referenced mutex
 *          If the mutex is locked the destruction is not done, this is an error
 * \param   mutex a pointer referencing a mutex 
 * \return  0 on success, 1 on fealure
 */
extern int pthread_mutex_destroy (pthread_mutex_t * mutex);


//--------------------------------------------------------------------------------------------------
// Barrier API
//--------------------------------------------------------------------------------------------------


/**
 * \brief   hidden barrier type, the user do not know what is in the barrier structure
 */
typedef struct pthread_barrier_s * pthread_barrier_t;

/**
 * \brief   hidden pthread barrier attribute type
 */
typedef struct pthread_barrierattr_s * pthread_barrierattr_t;

/**
 * \brief   creates or initialize a barrier depending the value of barrier parameter
 * \param   barrier a pointer referencing the barrier, there are two cases
 *                  1) if *barrier == NULL then allocate a new barrier, then initialize count
 *                  2) if *barrier != NULL it the barrier already exists, just initialize count 
 * \param   attr    a pointer to the attribute of the new barrier (NULL for default) 
 * \param   count   number of expected threads for this barrier
 * \return  SUCCESS if it all goes fine or
 *          EINVAL  The value specified by count is equal to zero.
 *          ENOMEM  Insufficient memory exists to initialize the barrier
 *          EBUSY   The implementation has detected an attempt to reinitialize a barrier 
 *                  while it is in use (for example, while being used in a  thread_barrier_wait()  
 *                  call) by  another thread.
 */
extern 
int pthread_barrier_init (pthread_barrier_t * barrier, pthread_barrierattr_t *attr, size_t count);

/**
 * \brief   wait to the referenced barrier, it is a blocking operation for all thread but the last
 *          the last arrived thread doesn't wait and notifies the other threads which becomes READY 
 * \param   barrier a pointer referencing a barrier 
 * \return  SUCCESS or FEALURE
 */
extern int pthread_barrier_wait (pthread_barrier_t * barrier);

/**
 * \brief   destroy the referenced barrier
 *          If a thread is waiting at the barrier, then the destruction is not done, it is an error
 * \param   barrier a pointer referencing a barrier 
 * \return  SUCCESS if it all goes fine or
 *          EINVAL  wrong argument
 *          EBUSY   The implementation has detected an attempt to destroy a barrier 
 *                  while it is in use (for example, while being used in a  thread_barrier_wait()  
 *                  call) by  another thread.
 */
extern int pthread_barrier_destroy (pthread_barrier_t * barrier);

#endif//_PTHREAD_H_
