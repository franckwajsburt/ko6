/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-10
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/chardev.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Generic CHARDEV functions prototypes

\*------------------------------------------------------------------------------------------------*/

#ifndef _HAL_CHARDEV_H_
#define _HAL_CHARDEV_H_

#include <common/errno.h>
#include <common/list.h>
#include <hal/dev.h>

#define CHARDEV_FIFO_DEPTH 20

struct chardev_s;

struct chardev_ops_s {
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

/**
 * Simple fifo (1 writer - 1 reader)
 *   - data      buffer of data
 *   - pt_write  write pointer for L fifos (0 at the beginning)
 *   - pt_read   read pointer for L fifos (0 at the beginning)
 *
 * data[] is used as a circular array. At the beginning (pt_read == pt_write) means an empty fifo
 * then when we push a data, we write it at pt_write, the we increment pt_write % fifo_size.
 * The fifo is full when it remains only one free cell, then when (pt_write + 1)%size == pt_read
 */
struct chardev_fifo_s {
    char data [CHARDEV_FIFO_DEPTH];
    unsigned pt_read;
    unsigned pt_write;
};

struct chardev_s {
    unsigned address;           // memory-mapped register addresses
    unsigned baudrate;          // chardev baudrate
    list_t list;                // linked-list entry into the chardev's list
    unsigned no;                // number of the chardev entry in the chardev's list
    struct chardev_fifo_s fifo;
    struct chardev_ops_s *ops;      // driver specific operations linked to the chardev
};

#define chardev_alloc() (struct chardev_s*) (dev_alloc(CHAR_DEV, sizeof(struct chardev_s))->data)
#define chardev_get(no) (struct chardev_s*) (dev_get(CHAR_DEV, no)->data)
#define chardev_count() (dev_next_no(CHAR_DEV) - 1)

/* Helper functions for CHARDEV's FIFOs */

/**
 * \brief   push a character into the chardev's FIFO
 * \param   fifo    structure of fifo to store data
 * \param   c       char to write
 * \return  SUCCESS or FAILURE
 */
int chardev_fifo_push (struct chardev_fifo_s *fifo, char c);

/**
 * \brief   pop a character from the chardev's FIFO
 * \param   fifo    structure of fifo to store data
 * \param   c       pointer on char to put the read char 
 * \return  SUCCESS or FAILURE
 */
int chardev_fifo_pull (struct chardev_fifo_s *fifo, int *c);

#endif