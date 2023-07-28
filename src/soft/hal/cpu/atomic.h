/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-11
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/cpu/atomic.h
  \author   Franck Wajsburt
  \brief    Generic cache functions prototypes

\*------------------------------------------------------------------------------------------------*/

#ifndef _HAL_CPU_ATOMIC_H_
#define _HAL_CPU_ATOMIC_H_

/**
 * Same as for the IRQs and some drivers, the function here
 * are marked extern because they need to be implemented at some
 * point by a platform-specific file
 */

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

#endif