/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-04
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     ulib/memory.h
  \author   Franck Wajsburt
  \brief    User memory allocator API 

\*------------------------------------------------------------------------------------------------*/

#ifndef _MALLOC_H_
#define _MALLOC_H_

/**
 * TODO: ask about this
*/
extern void sbrk_s(void);

/**
 * \brief   change the boundary of heap
 * \param   increment is the signed number of bytes , which will be aligned on a cache line 
 * \return  new end address of the heap, (increment=0) gives the current one, returns -1 on failure
 */
extern void * sbrk (int increment);

/**
 * \brief   print the heap state 
 * \param   level use: 0 = full/free bytes; 1 = the nb of blocks full/free per size; 2 = all details
 */
extern void malloc_print (size_t level);

/**
 * \brief   Heap initialiation, called only once before to call the main() fonction
 * \param   start address of the first byte  
 */
extern void malloc_init (void *start);

/**
 * \brief   Try to find a free object large enough, if none, merge free segments and try again.
 * \param   size  number of bytes asked
 * \return  A pointer of the allocated object or NULL if there is not place anymore.
 */
extern void * malloc(unsigned size);                 

/**
 * \brief   free an allocated object with  malloc(), object is not erased.
 * \param   ptr previously allocated object pointer
 */
extern void  free(void *ptr);    

#endif//_MALLOC_H_

