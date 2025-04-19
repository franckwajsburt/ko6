/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-14
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     hal/devices/chardev/soclib-tty.c
  \author   Franck Wajsburt, Nolan Bled
  \brief    Soclib TTY driver

\*------------------------------------------------------------------------------------------------*/

#include <hal/devices/chardev/soclib-tty.h>
#include <hal/cpu/irq.h>
#include <kernel/kthread.h>
#include <kernel/klibc.h>

/**
 * Soclib device registers
 */

struct soclib_tty_regs_s {
    int write;          ///< output to terminal
    int status;         ///< != 0 if something waiting in read register
    int read;           ///< input from keyboard
    int unused;         ///< no yet used
};

/**
 * \brief   Init the soclib tty device
 * \param   cdev soclib device
 * \param   base the soclib tty base address 
 * \param   baudrate the soclib tty baudrate (unused for this device)
 * \return  nothing
 */
static void soclib_tty_init(struct chardev_s *cdev, unsigned base, unsigned baudrate)
{
    cdev->ops       = &SoclibTTYOps;
    cdev->base      = base;
    cdev->baudrate  = baudrate;

    struct fifo_s *fifo = kmalloc(sizeof(struct fifo_s));
    cdev->driver_data = (void*) fifo;
}

/**
 * \brief   Read a buffer from the soclib tty of try to read a single char if count==0
 * \param   cdev the chardev device corresponding to the soclib tty
 * \param   buf the buf to fill
 * \param   count number of bytes to read from the tty
 *          it is a blocking function if count > 0 until count char is read
 *          if count == 0, it is a non-blocking function to read a single char
 * \return  if count > 0 then number of read bytes, else SUCCESS or FAILURE
 * FIXME    FIFO is not is the right place, it must be in the upper layer!
 */
static int soclib_tty_read(struct chardev_s *cdev, char *buf, unsigned count)
{
    struct fifo_s *fifo = (struct fifo_s *) cdev->driver_data;

    // blocking behavior
    if (count) {
        int res = 0;                                    // nb of read char
        char c;                                         // char read
        while (count--) {
            while (fifo_pull(fifo, &c) == FAILURE) {    // wait for a char from the keyboard
                thread_yield();                         // nothing then we yield the processor
                irq_enable();                           // get few characters if thread is alone
                irq_disable();                          // close enter
            }
            *buf++ = c;
            res++;
        }
        return res;                                     // return the number of char read
    }         
    // non-blocking behavior
    return fifo_pull(fifo, buf);                        // return SUCCESS or FAILURE
}

/**
 * \brief   Write in the TTY
 * \param   cdev the device struct corresponding to the tty
 * \param   buf the buffer to write
 * \param   count number of bytes to write
 * \return  number of bytes written
 */
static int soclib_tty_write(struct chardev_s *cdev, char *buf, unsigned count)
{
    int res = 0;                                        // nb of written char
    struct soclib_tty_regs_s *regs = 
        (struct soclib_tty_regs_s *) cdev->base;        // access the registers

    while (count--) {                                   // while there are chars
        regs->write = *buf;                             // send the char to TTY
        delay(150);
        res++;                                          // nb of written char
        buf++;		                                    // but is the next address in buffer
    }
    return res;
}

struct chardev_ops_s SoclibTTYOps = {
    .chardev_init = soclib_tty_init,
    .chardev_read = soclib_tty_read,
    .chardev_write = soclib_tty_write
};

void soclib_tty_isr(unsigned irq, struct chardev_s *cdev)
{
    struct soclib_tty_regs_s *regs = 
        (struct soclib_tty_regs_s *) cdev->base;
    
    struct fifo_s *fifo = (struct fifo_s *) cdev->driver_data;
    char c = regs->read;
    fifo_push(fifo, c);
}
/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
