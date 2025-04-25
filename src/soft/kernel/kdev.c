/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     kernel/kdev.c
  \author   Nolan Bled
  \brief    Generic device management functions

\*------------------------------------------------------------------------------------------------*/

#include <kernel/kdev.h>

static list_t DevList = {      // Do the same thing the function list_init does
    .next = &DevList,
    .prev = &DevList
};

unsigned dev_next_minor (dev_tag_t tag)
{
    list_foreach_rev (&DevList, item) {
        device_t *dev = list_item (item, device_t, list);
        if (dev->tag == tag)
            return dev->minor + 1;
    }
    return 0;
}

device_t *dev_alloc (dev_tag_t tag, unsigned dsize)
{
    /**
     * Allocate space for device metadata (tag, minor, list) and device-specific data (ops,...)
     */
    device_t *dev = kmalloc (sizeof(device_t) + dsize);  
    dev->tag = tag;
    dev->minor = dev_next_minor (tag);
    list_addlast (&DevList, &dev->list);
    return dev;
}

device_t *dev_get (dev_tag_t tag, unsigned minor)
{
    /**
     * Loop through the list until we find the corresponding entry
     * FIXME We could gain some performance by 
     * - having device-specific linked list but I can't think of a way to do it in a simple manner
     * - using a hash table indexed with a key formed with [minor<<3+tag]
     */
    list_foreach (&DevList, item) {
        device_t *dev = list_item (item, device_t, list);
        if (dev->tag == tag && dev->minor == minor)
            return dev;
    }
    return NULL;
}

void dev_free (device_t *dev)
{
    /**
     *  FIXME should we decrement every other device minor in the last, ex: should tty2 become tty1
     *  if tty0 is removed ? I don't think so but it could be interesting to think about it
     */
    list_unlink (&dev->list);
    kfree (dev);
}

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
