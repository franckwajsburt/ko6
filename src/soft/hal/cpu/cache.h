/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-11
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/cpu/cache.h
  \author   Franck Wajsburt
  \brief    Generic cache functions prototypes

\*------------------------------------------------------------------------------------------------*/

#ifndef _HAL_CPU_CACHE_H_
#define _HAL_CPU_CACHE_H_

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
 */
extern void dcache_buf_invalidate (void *buf, unsigned size);

/**
 * \brief   invalidate all line of the buffer buf 
 * \param   addr address to invalidate
 */
extern void dcache_invalidate (void *addr);

/**
 * \brief   get the value pointed by addr without use the cache
 *          this function must be used for all shared variables between several cpu
 * \return  the pointed value
 */
extern unsigned uncached_load (void * addr);

#endif