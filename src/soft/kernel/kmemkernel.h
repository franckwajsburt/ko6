/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     kernel/kmemkernel.h
  \author   Franck Wajsburt
  \brief    kernel allocators 

\*------------------------------------------------------------------------------------------------*/

#ifndef _KMEMKERNEL_H_
#define _KMEMKERNEL_H_

//--------------------------------------------------------------------------------------------------
// Page descriptor accessors
//--------------------------------------------------------------------------------------------------

/**
 * \brief Set the page type or Mark the given page as dirty / lock / valid.
 * \param page Pointer to the page buffer.
 */
void page_set_free (void *page);
void page_set_block (void *page);
void page_set_slab (void *page);
void page_set_valid (void *page);
void page_set_lock (void *page);
void page_set_dirty (void *page);

/**
 * \brief Clear the dirty / lock / valid flag of the given page.
 * \param page Pointer to the page buffer.
 */
void page_clr_valid (void *page);
void page_clr_lock (void *page);
void page_clr_dirty (void *page);

/**
 * \brief Test the given page type of if it is dirty / lock / valid.
 * \param page Pointer to the page buffer.
 * \return 1 if the page has the correct flag, 0 otherwise.
 */
int page_is_free (void *page);
int page_is_block (void *page);
int page_is_slab (void *page);
int page_is_valid (void *page);
int page_is_lock (void *page);
int page_is_dirty (void *page);

/**
 * \brief Increment/Decrement/Get the reference count of the given page.
 * \param page Pointer to the page buffer.
 * \return the new refcount value
 */
int page_inc_refcount (void *page);
int page_dec_refcount (void *page);
int page_get_refcount (void *page);

/**
 * \brief Set the Logical Block Address (BDEV,LBA) associated with this page.
 * \param page Pointer to the page buffer.
 * \param bdev Block device to associate.
 * \param lba Logical Block Address ioto associate.
 */
void page_set_lba (void *page, unsigned bdev, unsigned lba);

/**
 * \brief Get the Logical Block Address (BDEV,LBA) associated with this page.
 * \param page Pointer to the page buffer.
 * \param bdev address of Block device identifier used
 * \param lba address of Logical Block Address used
 * \return bdev and lba are copied at the address given
 */
void page_get_lba (void *page, unsigned *bdev, unsigned *lba);

//--------------------------------------------------------------------------------------------------

/**
 * \brief   initialize kernem memory allocators
 */
void kmemkernel_init (void);

/**
 * \brief   allocate an object in the kernel address space
 * \param   size in bytes (must be at most a PAGE SIZE)
 * \return  a pointer to an object with at least "size" byte size
 *          It is rounded up to a whole number of cache lines.
 *          The segment allocated is cleared because this is safer
 */
void * kmalloc (size_t size);

/**
 * \brief   Duplicates a string in kernel memory using the slab allocator.
 * \param   str   The null-terminated string to duplicate.
 * \return  A pointer to the newly allocated string, or NULL on failure.
 *          Returns NULL if allocation fails or if `str` is NULL.
 * \note    This function allocates memory using kmalloc () and copies the content of `str` into it.
 *          The caller is responsible for freeing the duplicated string using `kfree ()`.
 */
char *kstrdup (const char *str);

/**
 * \brief   same as kmalloc but allocate n * size bytes and write all the allocated zone to zero
 * \param   n     number of objects
 * \param   size  object size
 * \return  A pointer of the allocated object or NULL if there is not place anymore.
 */
extern void * kcalloc (size_t n, size_t size);                 

/**
 * \brief   free an allocated object with kmalloc ()
 * \param   obj pointer to the allocated object
 */
void kfree (void * obj);

//--------------------------------------------------------------------------------------------------

/**
 * \brief Print data about slab allocator usage.
 *        - for each open slab, tells the object size and how many free and allocated objects
 *        - for each page used, tells to which stab it belongs and how many objects are allocated in
 */
void kmalloc_stat (void);

/**
 * \brief Test Slab alocator, allocates and frees as many values as 'turn' then free all test
 * \param turn is the number of allocation or release are performed
 * \param size is the maximum objects size (from 1 to PAGE_SIZE)
 */
void kmalloc_test (size_t turn, size_t size);

#endif//_KMEMKERNEL_H_

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
