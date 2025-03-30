/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-03-30
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/devices/blockdev.h
  \author   Franck Wajsburt
  \brief    Generic BLOCKDEV functions prototypes

\*------------------------------------------------------------------------------------------------*/

#ifndef _HAL_BLOCKDEV_H_
#define _HAL_BLOCKDEV_H_

#include <common/errno.h>
#include <common/list.h>
#include <kernel/kdev.h>
#include <hal/cpu/atomic.h>

struct blockdev_ops_s;

#define LOGICAL_BLOCK_SIZE 4096

/** \brief Character device informations */
struct blockdev_s {
    unsigned base;                  ///< memory-mapped register base addresses
    unsigned blocks;                ///< Total number of available logical blocks (disk size) 
    unsigned block_size;            ///< Size (in bytes) of a logical block
    unsigned ppb;                   ///< physical blocks per logical block
    struct blockdev_ops_s *ops;     ///< driver specific operations linked to the blockdev
    void * driver_data;             ///< private pointer for driver specific info
};

/** 
 * \brief Functions prototypes of block device, they should be implemented by a device driver. 
 *        They serve as an interface between the kernel and the driver
 */
struct blockdev_ops_s {
    /**
     * \brief   Generic function that initialize the block device
     * \param   bdev        the block device
     * \param   base        base address of the device memory-mapped registers
     * \param   block_size  size of a logic block chosen by ko6
     */
    void (*blockdev_init)(struct blockdev_s *bdev, unsigned base, unsigned block_size);

    /**
     * \brief   Generic function that write to the block device
     * \param   bdev    the blockdev device
     * \param   buf     the buffer where to read the data to be written to the block device
     * \param   lba     the logic block address where the data is written
     * \param   count   the number of block to write
     * \return  number of blocks actually written
    */
    int (*blockdev_write)(struct blockdev_s *bdev, void *buf, unsigned lba, unsigned count);

    /**
     * \brief   Generic function that reads from the blockdev device
     * \param   bdev    the blockdev device
     * \param   buf     the buffer where the data is written
     * \param   lba     the logic block address where ito read the data to be written in buf
     * \param   count   the number of blocks to read
     * \return  number of blocks actually read
    */
    int (*blockdev_read)(struct blockdev_s *bdev, void *buf, unsigned lba, unsigned count);
};

#define blockdev_alloc() (struct blockdev_s*)(dev_alloc(BLOCK_DEV, sizeof(struct blockdev_s))->data)
#define blockdev_get(no) (struct blockdev_s*)(dev_get(BLOCK_DEV, no)->data)
#define blockdev_count() (dev_next_no(BLOCK_DEV) - 1)

#endif
