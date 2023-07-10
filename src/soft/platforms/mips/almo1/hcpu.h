/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-03
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/hcpu.h
  \author   Franck Wajsburt
  \brief    cpu API which is implemented by hcpuc.c & hcpua.S

\*------------------------------------------------------------------------------------------------*/


#ifndef _HCPU_H_
#define _HCPU_H_
#ifndef __ASSEMBLER__   /* the following lines are not included in assembler files */


//--------------------------------------------------------------------------------------------------
// Special registers 
//--------------------------------------------------------------------------------------------------


/**
 * \brief     clock cycle counter
 * \return    the number of cycles from the reset
 */
extern unsigned clock (void);

/** \brief  cpu identifier
 *  \return the current cpu identifier from 0 to NCPUS-1
 */
extern unsigned cpuid (void);

/**
 * \brief  prints all registers' value on TTY0 (must be in kernel mode) then stops program
 */
extern void kpanic (void);


//--------------------------------------------------------------------------------------------------
// L1 Cache operations
//--------------------------------------------------------------------------------------------------


/**
 * \brief   get the cache line size in bytes
 * \return  the size
 */
extern size_t cachelinesize (void);

/**
 * \brief   invalidate all line of the buffer buf 
 * \param   buf first address
 * \param   size of buffer in bytes
 * \return  nothing
 */
extern void dcache_buf_invalidate (void *buf, unsigned size);

/**
 * \brief   invalidate all line of the buffer buf 
 * \param   addr address to invalidate
 * \return  nothing
 */
extern void dcache_invalidate (void *addr);

/**
 * \brief   get the value pointed by addr without use the cache
 *          this function must be used for all shared variables between several cpu
 * \return  the pointed value
 */
extern unsigned uncached_load (void * addr);


//--------------------------------------------------------------------------------------------------
// Thread management
//--------------------------------------------------------------------------------------------------


/**
 * \brief   Initialize the thread context at the very beginning
 * \param   context       thread_context table 
 * \param   bootstrap     function without any argument, used just after the 1st thread_context_load
 * \param   stack_pointer top stack pointer at the very beginning
 * \return  side effect on the given thread context table
 */
extern void thread_context_init (int context[], void * bootstrap, void * stack_pointer);

/**
 * \brief   kernel saves the given thread registers in the context table
 * \param   context thread_context table of the thread to save
 * \return  it returns two values depending on how this function has been called.
 *          First case, the simplest, we call thread_save () then it returns 1 (true).
 *          Secondly, we call thread_restore() (see bellow) that causes the restoration
 *          of all registers recorded by thread_save(), including $31,
 *          thus when we return from thread_load(), in reality, we return from thread_save,
 *          but with 0 (false) as return value.
 */
extern int thread_context_save (int context[]);

/**
 * \brief   kernel load the given thread registers from the context table
 * \param   context thread_context table  of the thread to load (or to restore)
 * \return  returns alway 0 (false)
 */
extern int thread_context_load (int context[]);

/**
 * \brief   The kernel starts any thread for the very first time with this function.
 *          This function is called by the static function thread_bootstrap() (in kthread.c)
 *          which is the real beginning of a thread, and thread_bootstrap() is called by
 *          the first call of thread_load() for each thread (see comment of thread_bootstrat in
 *          kthread.c for more details)
 * \param   fun     ptr to the function of the thread, type:  void*(*fun)(void*) but cast to int
 * \param   arg     the argument given to fun(), type:   void*  but cast to int
 * \param   start   ptr to the user function which calls fun(arg) for real, type is cast to int
 *                  In fact, start has two possible types.
 *                  If it is the main thread then it is :
 *                      void (*start) (void)
 *                  but if it is a standard thread then it is :
 *                      void (*start) (void *(*fun) (void *), void *arg)
 * \return  never returns
 */
extern int thread_launch (int fun, int arg, int start);


//--------------------------------------------------------------------------------------------------
// IRQ mask operations 
//--------------------------------------------------------------------------------------------------


/** 
 * \brief   enable irq (do not change the MIPS mode thus stay in kernel mode)
 * \return  nothing
 */
extern void irq_enable (void);

/** 
 * \brief   disable irq 
 * \return  the previous state
 */
extern unsigned irq_disable (void);

/** 
 * \brief   restore an irq state got from disable irq
 * \return  nothing
 */
extern void irq_retore (unsigned state);


//--------------------------------------------------------------------------------------------------
// spin and atomic operations 
//--------------------------------------------------------------------------------------------------


/**
 * It's a simple word, but it is forbidden to read it directly to not copy it in cache
 */
typedef unsigned spinlock_t;

/** 
 * \brief   get the lock, block until success
 * \param   lock blocking function until lock is free 
 */
extern void spin_lock (spinlock_t * lock);

/** 
 * \brief   release the lock 
 * \param   lock freeing the lock
 */
extern void spin_unlock (spinlock_t * lock);

/** 
 * \brief   add a value to the pointed value, block until success
 * \param   counter pointer to change
 * \param   val value to add
 * \return  the new value
 */
extern int atomic_add (int * counter, int val);


#endif//__ASSEMBLER__
#endif//_HCPU_H_


//--------------------------------------------------------------------------------------------------
// include specific definitions for the current SoC
//--------------------------------------------------------------------------------------------------


#include <hcpu_soc.h>
