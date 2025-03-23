/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-03
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     kernel/ksynchro.h
  \author   Franck Wajsburt
  \brief    synchronization service function API for the user application

\*------------------------------------------------------------------------------------------------*/


#ifndef _KSYNC_H_
#define _KSYNC_H_

//--------------------------------------------------------------------------------------------------
// Global function
//--------------------------------------------------------------------------------------------------


/**
 * \brief   Initialize global lists, and maybe other things...
 */
int ksynchro_init (void);


//--------------------------------------------------------------------------------------------------
// Mutex API
//--------------------------------------------------------------------------------------------------


/**
 * \brief   hidden mutex type, the other modules do not what is in the mutex structure
 */
typedef struct thread_mutex_s * thread_mutex_t;

/**
 * \brief   creates a new mutex and initializes mutex variable with the new mutex (side effect)
 *          it is a nutex with error checking 
 * \param   mutex a pointer referencing the new mutex 
 * \return  0 on success, 1 on fealure
 */
extern int thread_mutex_init (thread_mutex_t * mutex);

/**
 * \brief   destroy the referenced mutex
 *          If the mutex is locked the destruction is not done, this is an error
 * \param   mutex a pointer referencing a mutex 
 * \return  0 on success, 1 on fealure
 */
extern int thread_mutex_destroy (thread_mutex_t * mutex);

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
 * \brief   delete all mutexes from a given pid
 * \param   pid the process identifier that owns the mutexes
 * \return  0 on success, 1 on fealure
 */
extern int process_mutexes_cleanup (int pid);


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
 *          EINVAL  barrier is not a barrier or the value specified by count is equal to zero.
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

/**
 * \brief   delete all barriers from a given pid
 * \param   pid the process identifier that owns the barriers
 * \return  0 on success, 1 on fealure
 */
extern int process_barriers_cleanup (int pid);



#endif//_KSYNC_H_
