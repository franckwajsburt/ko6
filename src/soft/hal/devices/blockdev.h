/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-24
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     hal/devices/blockdev.h
  \author   Franck Wajsburt
  \brief    Generic BLOCKDEV functions prototypes

\*------------------------------------------------------------------------------------------------*/

#ifndef _HAL_BLOCKDEV_H_
#define _HAL_BLOCKDEV_H_

struct blockdev_ops_s;

#define LOGICAL_BLOCK_SIZE 4096

/** \brief Structure describing what to do when we receive a bd interrupt */
struct blockdev_event_s {
    void (*fn)(void *arg,int status);///< function triggered
    void *arg;                      ///< argument passed to the function
};

/** \brief block device informations */
typedef struct blockdev_s {
    unsigned base;                  ///< memory-mapped register base addresses
    unsigned minor;                 ///< device identifier MINOR number
    unsigned blocks;                ///< Total number of available logical blocks (disk size) 
    unsigned block_size;            ///< Size (in bytes) of a logical block
    unsigned ppb;                   ///< physical blocks per logical block
    struct blockdev_event_s event;  ///< event triggered each nticks
    struct blockdev_ops_s *ops;     ///< driver specific operations linked to the blockdev
    void * driver_data;             ///< private pointer for driver specific info
} blockdev_t;

/** 
 * \brief Functions prototypes of block device, they should be implemented by a device driver. 
 *        They serve as an interface between the kernel and the driver
 */
struct blockdev_ops_s {
    /**
     * \brief   Generic function that initialize the block device
     * \param   bdev        the block device
     * \param   minor       minor number is the device instance number
     * \param   base        base address of the device memory-mapped registers
     * \param   block_size  size of a logic block chosen by ko6
     * \note    almo1-mips : soclib_bd_init
     */
    void (*blockdev_init)(blockdev_t *bdev, unsigned minor, unsigned base, unsigned block_size);

    /**
     * \brief   Generic function that write to the block device
     * \param   bdev    the blockdev device
     * \param   lba     the logic block address where the data is written
     * \param   buf     the buffer where to read the data to be written to the block device
     * \param   count   the number of block to write
     * \return  0 on success, -EINVAL if invalid arguments
     * \note    almo1-mips : soclib_bd_write
     */
    int (*blockdev_write)(blockdev_t *bdev, unsigned lba, void *buf, unsigned count);

    /**
     * \brief   Generic function that reads from the blockdev device 
     *          almo1-mips : soclib_bd_read
     * \param   bdev    the blockdev device
     * \param   lba     the logic block address where ito read the data to be written in buf
     * \param   buf     the buffer where the data is written
     * \param   count   the number of blocks to read
     * \return  0 on success, -EINVAL if invalid arguments
     * \note    almo1-mips : soclib_bd_read
     */
    int (*blockdev_read)(blockdev_t *bdev, unsigned lba, void *buf, unsigned count);

    /**
     * \brief   Set    the event that will triggered by a block device interrupt
     * \param   bdev   the block device
     * \param   f      the function corresponding to the event
     * \param   arg    argument that will be passed to the function
     * \param   status the block device status once the command is done
     * \note    almo1-mips : soclib_bd_event
     */
    void (*blockdev_set_event)(blockdev_t *bdev, void(*f)(void *arg, int status), void *arg);
};

#endif

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
