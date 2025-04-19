/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-19
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     ulib/memory.h
  \author   Franck Wajsburt
  \brief    User memory allocator API 

\*------------------------------------------------------------------------------------------------*/

#ifndef _MALLOC_H_
#define _MALLOC_H_

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
extern void malloc_print (int level);

/**
 * \brief   Heap initialiation, called only once before to call the main() fonction
 * \param   start address of the first byte  
 */
extern void malloc_init (void *start);

/**
 * \brief   Try to allocate an object large enough, if none, merge free segments and try again.
 * \param   size  number of bytes asked
 * \return  A pointer of the allocated object or NULL if there is not place anymore.
 */
extern void * malloc(unsigned size);                 

/**
 * \brief   same as malloc but allocate n * size bytes and write all the allocated zone to zero
 * \param   n     number of objects
 * \param   size  object size
 * \return  A pointer of the allocated object or NULL if there is not place anymore.
 */
extern void * calloc(size_t n, size_t size);                 

/**
 * \brief   free an allocated object with  malloc(), object is not erased.
 * \param   ptr previously allocated object pointer
 */
extern void  free(void *ptr);    

/**
 * \brief   Duplicates a string in memory using the standardd allocator.
 * \param   str   The null-terminated string to duplicate.
 * \return  A pointer to the newly allocated string, or NULL on failure.
 *          Returns NULL if allocation fails or if `str` is NULL.
 * \note    This function allocates memory using malloc() and copies the content of `str` into it.
 *          The caller is responsible for freeing the duplicated string using `free()`.
 */
char *strdup(const char *str);

#endif//_MALLOC_H_

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
