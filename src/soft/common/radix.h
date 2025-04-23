/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     common/radix.h
  \author   Franck Wajsburt
  \brief    A radix tree is a one-dimensional table, it is equivalent to: Type radix [ ELEMENTS ]
            where:  Type        is a native C type (int, short, void *)
                    NBELEMENTS  is the number of slots
            There:  Type        void *
                    NBELEMENTS  2**32 = 4 Giga slots

\*------------------------------------------------------------------------------------------------*/

#ifndef _RADIX_H_
#define _RADIX_H_

#ifdef _HOST_
#   include <stdlib.h>
#   include <stdio.h>
#   include <string.h>
#elif defined  _KERNEL_
#   include <kernel/klibc.h>
#else
#   include <libc.h>
#endif

/**
 * \brief   Opaque structure representing a radix tree
 */
typedef struct radix_s radix_t;

/**
 * \brief   Creates a new radix tree 
 * \return  A pointer to the newly allocated radix tree, or NULL if allocation fails.
 */
radix_t * radix_create (void);

/**
 * \brief   Retrieves the value placed at a given index in the radix_tree.
 * \param   radix   Pointer to the radix tree
 * \param   index   position where the data is searched
 * \return  the value in radix [ index ]
 *          If nothing has been written in this location yet, it simply returns NULL
 */
void * radix_get (const radix_t *radix, unsigned index);

/**
 * \brief   Inserts a value into the radix tree.
 * \param   radix   Pointer to the radix tree
 * \param   index   position where the data has to be written
 * \param   val     The value to set
 * \return  SUCCESS or FEALURE
 *          radix_set may need to allocate memory, so it may fail if there is no more memory
 */
int radix_set (radix_t *radix, unsigned index, void * val);

/**
 * \brief   Function pointer type for callbacks used in `radix_foreach`.
 *          A callback function of this type is executed for each occupied slot in the radix tree. 
 *          It receives the slot index, the associated value and a user-defined data ptr
 * \param   radix Pointer to the radix tree.
 * \param   index The index of the slot in the radix tree.
 * \param   val   The value associated with the key.
 * \param   data  A user-defined pointer passed to provide context to the callback function.
 */
typedef void (*radix_callback_t)(const radix_t *radix, unsigned index, void *val, void *data);

/**
 * \brief   Iterates over all occupied slots in the radix tree and applies a callback function.
 *          This function traverses all slots in the radix tree and calls the given callback 
 *          function on each occupied slot (i.e., slots that contain a no-NULL value). 
 *          The callback function follows the `radix_callback_t` signature (see above)
 * \param   radix Pointer to the radix tree.
 * \param   fn    Function of type `radix_callback_t` that will be called for each valid slot.
 * \param   data  A user-defined pointer passed to the callback function for additional context.
 *          The callback function is called only for valid entries (not NULL).
 */
void radix_foreach(const radix_t *radix, radix_callback_t fn, void *data);

/**
 * \brief   cleanup a radix tree
 * \param   radix Pointer to the radix tree.
 * \return  Recursively frees empty nodes in the radix tree.
 *           1) free all allocated node whose all slots are NULL
 *           2) set to NULL the parent pointer if the child node is freed
 *              Also clears root_l1/l2/l3 if their respective subtrees become empty.
 */
void radix_cleanup(radix_t *rx);

/**
 * \brief   Displays statistics about the radix tree.
 *          this function prints the stucture.
 * \param   radix  Pointer to the radix tree.
 */
void radix_stat (const radix_t *radix);

extern void radix_export_dot(const radix_t *rx, const char *filename);

#endif

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
