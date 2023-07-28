/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-03
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
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
 * \param   size in bytes used for allocation
 */
void kfree (void * obj, size_t size);

/**
 * \brief Print data about slab allocator usage.
 *        - for each open slab, tells the object size and how many free and allocated objects
 *        - for each page used, tells to which stab it belongs and how many objects are allocated in
 */
void kmalloc_print (void);

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
 * \param   addr  is the new boundary, maybe greater or smaller than the current one
 * \return  SUCCESS or FAILURE 
 */
void * sbrk (int increment);

#endif//_ALLOC_H_
