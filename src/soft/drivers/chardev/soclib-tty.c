/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-10
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     drivers/chardev/soclib-tty.c
  \author   Franck Wajsburt, Nolan Bled
  \brief    Soclib TTY driver

\*------------------------------------------------------------------------------------------------*/

#include <drivers/chardev/soclib-tty.h>

/**
 * \brief   Init the soclib tty device
 * \param   cdev soclib device
 * \param   address the soclib tty address 
 * \param   baudrate the soclib tty baudrate (unused for this device)
 * \return  nothing
 */
static void soclib_tty_init(struct chardev_s *cdev, unsigned address, unsigned baudrate)
{
    cdev->ops        = &SoclibTTYOps;
    cdev->address    = address;
    cdev->baudrate   = baudrate;
}

/**
 * \brief   Read a buffer from the soclib tty
 * \param   cdev the chardev device corresponding to the soclib tty
 * \param   buf the buf to fill
 * \param   count number of bytes to read from the tty
 * \return  number of bytes written
 */
static int soclib_tty_read(struct chardev_s *cdev, char *buf, unsigned count)
{
    int res = 0;                                        // nb of read char
    int c;                                              // char read

    while (count--) {
        while (chardev_fifo_pull(&cdev->fifo, &c) == FAILURE) {  // wait for a char from the keyboard
            thread_yield();                                 // nothing then we yield the processor
            irq_enable();                                   // get few characters if thread is alone
            irq_disable();                                  // close enter
        }
        *buf++ = c;
        res++;
    }
    return res;                                         // return the number of char read
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
        (struct soclib_tty_regs_s *) cdev->address;     // access the registers

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
        (struct soclib_tty_regs_s *) cdev->address;
    char c = regs->read;
    chardev_fifo_push(&cdev->fifo, c);
}
