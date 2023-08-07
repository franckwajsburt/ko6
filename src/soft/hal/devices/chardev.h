/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-10
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/arch/chardev.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Generic CHARDEV functions prototypes

\*------------------------------------------------------------------------------------------------*/

#ifndef _HAL_CHARDEV_H_
#define _HAL_CHARDEV_H_

#include <common/errno.h>
#include <common/list.h>
#include <kernel/kdev.h>
#include <hal/cpu/atomic.h>

struct chardev_s;

/** 
 * \brief Functions prototypes of character device, they should be implemented
 *        by a device driver. They serve as an interface between the kernel and
 *        the driver
 */
struct chardev_ops_s {
    /**
     * \brief   Generic function that initialize the chardev device
     * \param   chardev the char device
     * \param   address base address of the device memory-mapped registers 
     * \param   baudrate the device baudrate
     */
    void (*chardev_init)(struct chardev_s *chardev, unsigned address, unsigned baudrate);

    /**
     * \brief   Generic function that write to the chardev device
     * \param   chardev     the chardev device
     * \param   buf     the buffer to write to the chardev
     * \param   count   the number of bytes to write
     * \return  number of bytes actually written
    */
    int (*chardev_write)(struct chardev_s *chardev, char *buf, unsigned count);

    /**
     * \brief   Generic function that reads from the chardev device
     * \param   chardev     the chardev device
     * \param   buf     the buffer that will receive the data from the chardev
     * \param   count   the number of bytes that should be written into the buffer
     * \return  number of bytes actually read
    */
    int (*chardev_read)(struct chardev_s *chardev, char *buf, unsigned count);
};

/** \brief Character device informations */
struct chardev_s {
    unsigned address;               ///< memory-mapped register addresses
    unsigned baudrate;              ///< chardev baudrate
    struct chardev_ops_s *ops;      ///< driver specific operations linked to the chardev
    void * driver_data;             ///< private pointer for driver specific info
};

#define chardev_alloc() (struct chardev_s*) (dev_alloc(CHAR_DEV, sizeof(struct chardev_s))->data)
#define chardev_get(no) (struct chardev_s*) (dev_get(CHAR_DEV, no)->data)
#define chardev_count() (dev_next_no(CHAR_DEV) - 1)

#endif
