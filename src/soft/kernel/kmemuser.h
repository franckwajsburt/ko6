/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-13
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     kernel/kmemory.h
  \author   Franck Wajsburt
  \brief    kernel allocators and user memory management API

\*------------------------------------------------------------------------------------------------*/

#ifndef _KMEMUSER_H_
#define _KMEMUSER_H_

/**
 * \brief   initialize user memory allocators
 */
void kmemuser_init (void);

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

#endif//_KMEMUSER_H_

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
