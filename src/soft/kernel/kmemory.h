/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-02-23
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     kernel/kmemory.h
  \author   Franck Wajsburt
  \brief    kernel allocators and user memory management API

\*------------------------------------------------------------------------------------------------*/

#ifndef _ALLOC_H_
#define _ALLOC_H_

#include <common/usermem.h>

/**
 * \brief   initialize all the kernel memory allocators
 */
void memory_init (void);

/**
 * \brief   allocate an object in the kernel address space
 * \param   size in bytes (must be at most a PAGE SIZE)
 * \return  a pointer to an object with at least "size" byte size
 *          It is rounded up to a whole number of cache lines.
 */
void * kmalloc (size_t size);

/**
 * \brief   free an allocated object with kmalloc()
 * \param   obj pointer to the allocated object
 */
void kfree (void * obj);

/**
 * \brief   Duplicates a string in kernel memory using the slab allocator.
 * \param   str   The null-terminated string to duplicate.
 * \return  A pointer to the newly allocated string, or NULL on failure.
 *          Returns NULL if allocation fails or if `str` is NULL.
 * \note    This function allocates memory using kmalloc() and copies the content of `str` into it.
 *          The caller is responsible for freeing the duplicated string using `kfree()`.
 */
char *kstrdup(const char *str);

/**
 * \brief Print data about slab allocator usage.
 *        - for each open slab, tells the object size and how many free and allocated objects
 *        - for each page used, tells to which stab it belongs and how many objects are allocated in
 */
void kmalloc_stat (void);

/**
 * \brief   same as kmalloc but allocate n * size bytes and write all the allocated zone to zero
 * \param   n     number of objects
 * \param   size  object size
 * \return  A pointer of the allocated object or NULL if there is not place anymore.
 */
extern void * kcalloc(size_t n, size_t size);                 

/**
 * \brief Test Slab alocator, allocates and frees as many values as 'turn' then free all test
 * \param turn is the number of allocation or release are performed
 * \param size is the maximum objects size (from 1 to PAGE_SIZE)
 */
void kmalloc_test (size_t turn, size_t size);

/**
 * \brief   allocate a new user stack
 * \return  a pointer to an empty stack (of STACK_SIZE bytes)
 *          The pointer is just above the new stack
 */
int * malloc_ustack (void);

/**
 * \brief   free a use stack
 * \param   stack is a pointer returned by kmalloc_ustack
 */
void free_ustack (int * stack);

/**
 * \brief   print ustack state
 */
void print_ustack (void);
void test_ustack (size_t turn);

/**
 * \brief   change the boundary of the heap
 * \param   increment integer added to the current heap boundary
 * \return  SUCCESS or FAILURE
 */
void * sbrk (int increment);

#endif//_ALLOC_H_
