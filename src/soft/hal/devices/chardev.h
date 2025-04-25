/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-24
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     hal/devices/chardev.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Generic CHARDEV functions prototypes

\*------------------------------------------------------------------------------------------------*/

#ifndef _HAL_CHARDEV_H_
#define _HAL_CHARDEV_H_

struct chardev_ops_s;

/** \brief Character device informations */
typedef struct chardev_s {
    unsigned base;                  ///< memory-mapped register base addresses
    unsigned minor;                 ///< device identifier MINOR number
    unsigned baudrate;              ///< chardev baudrate
    struct chardev_ops_s *ops;      ///< driver specific operations linked to the chardev
    void * driver_data;             ///< private pointer for driver specific info
} chardev_t;

/** 
 * \brief Functions prototypes of character device, they should be implemented by a device driver. 
 *        They serve as an interface between the kernel and the driver
 */
struct chardev_ops_s {
    /**
     * \brief   Generic function that initialize the chardev device
     * \param   chardev the char device
     * \param   minor   minor number is the device instance number
     * \param   base    base address of the device memory-mapped registers 
     * \param   baudrate the device baudrate
     * \note    almo1-mips : soclib_tty_init
     */
    void (*chardev_init)(chardev_t *chardev, unsigned minor, unsigned base, unsigned baudrate);

    /**
     * \brief   Generic function that write to the chardev device
     * \param   chardev     the chardev device
     * \param   buf     the buffer to write to the chardev
     * \param   count   the number of bytes to write
     * \return  number onf bytes actually written
     * \note    almo1-mips : soclib_tty_write
    */
    int (*chardev_write)(chardev_t *chardev, char *buf, unsigned count);

    /**
     * \brief   Generic function that reads from the chardev device
     * \param   chardev     the chardev device
     * \param   buf     the buffer that will receive the data from the chardev
     * \param   count   the number of bytes that should be written into the buffer
     * \return  number of bytes actually read
     * \note    almo1-mips : soclib_tty_read
     */
    int (*chardev_read)(chardev_t *chardev, char *buf, unsigned count);
};

#endif

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
