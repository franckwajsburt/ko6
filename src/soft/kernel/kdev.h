/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-25
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     kernel/kdev.h
  \author   Nolan Bled
  \brief    Generic device management functions

  To simplify device management, this module provides functions and structures to dynamically
  allocate and retrieve devices. During platform initialization (typically from soc_init()
  in hal/soc/xxx/soc.c), the required devices are allocated and registered.

  Each device is stored in the kernel heap and linked into a global list (DevList).
  Every device has a tag indicating its type (CHAR_DEV, BLOCK_DEV, ICU_DEV, etc.).
  For each type, a local identifier number is assigned, starting from zero and incremented
  with each new instance. This identifier is commonly referred to as the minor number,
  by analogy with UNIX-style device management.

  Each device in the global list is represented by a small generic descriptor (dev_s),
  and dynamically extended with a driver-specific structure (see hal/devices/xxx.h).
  This extended structure is allocated together with the device and includes, for example:
  * The memory-mapped base address of the device
  * The associated IRQ number (if applicable)
  * A pointer to the driver’s function table (ops)
  * Other device-specific fields
  This structure is exclusively manipulated by the corresponding driver and is accessible
  through the data field of the generic dev_s descriptor.

  Example of a DevList:
  * When the OS starts we have:
               devList
        ┌────────────────┐
        │.next = devList │
        │.prev = devList │
        └────────────────┘
  * We then allocate a TTY: dev_alloc (TTY_DEV, sizeof(struct tty_s))
               devList                tty0
        ┌────────────────┐     ┌────────────────┐
        │.next = tty0    ├─────►.next = devList │
        │.prev = tty0    ◄─────┤.prev = devList │
        └────────────────┘     ├────────────────┤
                               │.tag  = CHAR_DEV│
                               │.minor= 0       │
                               ├────────────────┤
                               │.data           │
                               │ (chardev_t *)  │
                               └────────────────┘
  * Then an ICU: dev_alloc (ICU_DEV, sizeof(struct icu_s))
               devList                tty0                    icu0
        ┌────────────────┐     ┌────────────────┐     ┌────────────────┐
        │.next = tty0    ├─────►.next = icu0    ├─────►.next = devList │
        │.prev = icu0    ◄─────┤.prev = devList ◄─────┤.prev = tty0    │
        └────────────────┘     ├────────────────┤     ├────────────────┤
                               │.tag  = CHAR_DEV│     │.tag  = ICU_DEV │
                               │.minor= 0       │     │.minor= 0       │
                               ├────────────────┤     ├────────────────┤
                               │.data           │     │.data           │
                               │ (chardev_t *)  │     │ (icu_t *)      │
                               └────────────────┘     └────────────────┘
  * And another TTY: dev_alloc (TTY_DEV, sizeof(struct tty_s))
               devList                tty0                    icu0                   tty1
        ┌────────────────┐     ┌────────────────┐     ┌────────────────┐     ┌────────────────┐
        │.next = tty0    ├─────►.next = icu0    ├─────►.next = tty1    ├─────►.next = devList │
        │.prev = icu0    ◄─────┤.prev = devList ◄─────┤.prev = tty0    ◄─────┤.prev = icu0    │
        └────────────────┘     ├────────────────┤     ├────────────────┤     ├────────────────┤
                               │.tag  = CHAR_DEV│     │.tag  = ICU_DEV │     │.tag  = CHAR_DEV│
                               │.minor= 0       │     │.minor= 0       │     │.minor= 1       │
                               ├────────────────┤     ├────────────────┤     ├────────────────┤
                               │.data           │     │.data           │     │.data           │
                               │ (chardev_t *)  │     │ (icu_t *)      │     │ (chardev_t *)  │
                               └────────────────┘     └────────────────┘     └────────────────┘

  In traditional UNIX-like systems such as Linux, devices are identified by a pair: (major, minor).
  * The major number identifies a family of devices that share the same driver interface
    (i.e., the same set of operations).
  * The minor number identifies an instance handled by that driver.

  For example, all character devices implement the same standard API (open, read, write, release, 
  etc.). But each major number corresponds to a different implementation of that API, such as 
  a UART, a keyboard, or a pseudo-terminal.

  The same applies to block devices (blockdev): all use a common interface (read_block, write_block,
  flush, etc.), while the specific implementation depends on the driver associated with the major 
  number (e.g., RAM disk, SD card, or hardware disk controller).

  In ko6, the major number is not explicitly stored in the device structure.
  However, each device type (blockdev, chardev, etc.) maintains its own namespace for minor numbers.
  As in Linux, all devices of the same type expose the same API (blockdev_ops_s, chardev_ops_s, ...),
  but each instance can have a different implementation of that API.

\*------------------------------------------------------------------------------------------------*/

#ifndef _KERNEL_DEV_H_
#define _KERNEL_DEV_H_

#include <common/list.h>
#include <kernel/klibc.h>

/** \brief Devices Types */
typedef enum dev_tag_e {
    BLOCK_DEV,
    CHAR_DEV,
    ICU_DEV,
    DMA_DEV,
    TIMER_DEV
} dev_tag_t;

/** \brief Device Driver informations
 */
typedef struct dev_s {
    dev_tag_t tag;      ///< Identify the type of the device (tty, icu, ...)
    unsigned minor;     ///< Minor device number (tty-1, tty1, ...)
    list_t list;        ///< List entry in the global device list
    char data[];        ///< Device specific data, to be filled in with (struct tty_s, icu_s, ...)
} dev_t;

/**
 * \brief   Find the last element added with corresponding tag
 *          To do so, we loop through the device list in a reverse order
 *          Once we found the last device, we add one to his number
 * \param   tag type of the device (tty, icu, ...)
 * \return  the next minor (last device of this type minor + 1)
*/
extern unsigned dev_next_minor (dev_tag_t tag);

/**
 * \brief   Allocate enough size in kernel heap to store device meta data (tag, minor, list)
 *          and device data (struct tty_s, struct icu_s, ...) and add it into the global device
 *          list
 * \param   tag type of the device (tty, icu, ...)
 * \param   dsize size of the device-specific structure (ex: sizeof (struct_s))
 * \return  the allocated device
*/
extern struct dev_s *dev_alloc (dev_tag_t tag, unsigned dsize);

/**
 * \brief   Get a device based on its type and on its minor number
 * \param   tag   type of the device
 * \param   minor minor number of the device
 * \return  the corresponding device if found, NULL if not
*/
extern struct dev_s *dev_get (dev_tag_t tag, unsigned minor);

/**
 * \brief   Release a created device (kfree + list_unlink)
 * \param   dev the device to release
*/
extern void dev_free (struct dev_s *dev);

//--------------------------------------------------------------------------------------------------
// Device helpers
// data is the physical device register map
//--------------------------------------------------------------------------------------------------

#define blockdev_get(minor)   (blockdev_t*)(dev_get(BLOCK_DEV, minor)->data)
#define chardev_get(minor)    (chardev_t *)(dev_get(CHAR_DEV,  minor)->data)
#define timerdev_get(minor)   (timer_t   *)(dev_get(TIMER_DEV, minor)->data)
#define dmadev_get(minor)     (dma_t     *)(dev_get(DMA_DEV,   minor)->data)
#define icudev_get(minor)     (icu_t     *)(dev_get(ICU_DEV,   minor)->data)

#define blockdev_count(minor) (dev_next_minor(BLOCK_DEV)-1)
#define chardev_count(minor)  (dev_next_minor(CHAR_DEV) -1)
#define timerdev_count(minor) (dev_next_minor(TIMER_DEV)-1)
#define dmadev_count(minor)   (dev_next_minor(DMA_DEV)  -1)
#define icudev_count(minor)   (dev_next_minor(ICU_DEV)  -1)

#endif
