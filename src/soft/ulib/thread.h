/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-04
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     ulib/thread.h
  \author   Franck Wajsburt
  \brief    Thread API

\*------------------------------------------------------------------------------------------------*/


#ifndef _THREAD_H_
#define _THREAD_H_


//--------------------------------------------------------------------------------------------------
// Thread API
//--------------------------------------------------------------------------------------------------


/**
 * \brief   hidden thread type, the user do not what is in the thread structure
 */
typedef struct thread_s * thread_t;

/**
 * \brief   asks the kernel via a syscall to start a new thread by invoking fun(arg)
 *          fun() can stop the thread by calling thread_exit(value)
 *          or if it returns a value, it is equivalent to call thread_exit with this value
 *          the user implementation of thread_create uses a syscall with 4 arguments:
 *          the three of thread_create (thread, fun, arg) plus a fourth which is thread_starter()
 *          The state of the new thread is TH_STATE_READY.
 *
 * \param   thread  pointer to the thread structure
 * \param   fun     pointer to the function of the thread
 * \param   arg     the argument given to fun()
 */
extern int thread_create (thread_t * thread, void *(*fun) (void *), void *arg);

/**
 * \brief   Causes the current thread to give up the CPU in order to give it to another
 */
extern int thread_yield (void);

/**
 * \brief   terminates the current thread and returns retval
 * \param   retval the value can be retrieved by calling the thread_join function of another thread
 */
extern void thread_exit (void *retval);

/**
 * \brief   wait for a thread termination
 * \param   thread  expected thread 
 * \param   retval  pointer to the return value of the expected thread
 */
extern void thread_join (thread_t thread, void **retval);

/**
 * \brief   Displays on the console (tty0) all active threads, it is for debugging.
 */
void sched_dump(void);


//--------------------------------------------------------------------------------------------------
// Mutex API
//--------------------------------------------------------------------------------------------------


/**
 * \brief   hidden mutex type, the user do not what is in the mutex structure
 */
typedef struct thread_mutex_s * thread_mutex_t;

/**
 * \brief   creates a new mutex and initializes mutex variable with the new mutex (side effect)
 *          it is a mutex with error checking 
 * \param   mutex a pointer referencing the new mutex 
 * \return  0 on success, 1 on fealure
 */
extern int thread_mutex_init (thread_mutex_t * mutex);

/**
 * \brief   lock the referenced mutex, it is a blocking operation.
 *          If the mutex is already locked, the calling thread blocks until the mutex becomes 
 *          available. If the mutex does not exist or is already locked, this is an error
 * \param   mutex a pointer referencing a mutex 
 * \return  0 on success, 1 on fealure
 */
extern int thread_mutex_lock (thread_mutex_t * mutex);

/**
 * \brief   unlock the referenced mutex
 *          If the mutex does not exist or is not locked or has been locked by another, 
 *          this is an error that causes exit with error
 * \param   mutex a pointer referencing a mutex 
 * \return  0 on success, 1 on fealure
 */
extern int thread_mutex_unlock (thread_mutex_t * mutex);

/**
 * \brief   destroy the referenced mutex
 *          If the mutex is locked the destruction is not done, this is an error
 * \param   mutex a pointer referencing a mutex 
 * \return  0 on success, 1 on fealure
 */
extern int thread_mutex_destroy (thread_mutex_t * mutex);


//--------------------------------------------------------------------------------------------------
// Barrier API
//--------------------------------------------------------------------------------------------------


/**
 * \brief   hidden barrier type, the user do not what is in the barrier structure
 */
typedef struct thread_barrier_s * thread_barrier_t;

/**
 * \brief   creates or initialize a barrier depending the value of barrier parameter
 * \param   barrier a pointer referencing the barrier, there are two cases
 *                  1) if *barrier == NULL then allocate a new barrier, then initialize count
 *                  2) if *barrier != NULL it the barrier already exists, just initialize count 
 * \param   count   number of expected threads for this barrier
 * \return  SUCCESS if it all goes fine or
 *          EINVAL  The value specified by count is equal to zero.
 *          ENOMEM  Insufficient memory exists to initialize the barrier
 *          EBUSY   The implementation has detected an attempt to reinitialize a barrier 
 *                  while it is in use (for example, while being used in a  thread_barrier_wait()  
 *                  call) by  another thread.
 */
extern int thread_barrier_init (thread_barrier_t * barrier, size_t count);

/**
 * \brief   wait to the referenced barrier, it is a blocking operation for all thread but the last
 *          the last arrived thread doesn't wait and notifies the other threads which becomes READY 
 * \param   barrier a pointer referencing a barrier 
 * \return  SUCCESS or FEALURE
 */
extern int thread_barrier_wait (thread_barrier_t * barrier);

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
extern int thread_barrier_destroy (thread_barrier_t * barrier);

#endif//_THREAD_H_
