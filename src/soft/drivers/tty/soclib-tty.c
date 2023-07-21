/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-10
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     drivers/tty/soclib-tty.c
  \author   Franck Wajsburt, Nolan Bled
  \brief    Soclib TTY driver

\*------------------------------------------------------------------------------------------------*/

#include <drivers/tty/soclib-tty.h>

static void soclib_tty_init(struct tty_s *tty, unsigned address, unsigned baudrate)
{
    tty->ops        = &soclib_tty_ops;
    tty->address    = address;
    tty->baudrate   = baudrate;
}

static int soclib_tty_read(struct tty_s *tty, char *buf, unsigned count)
{
    int res = 0;                                        // nb of read char
    int c;                                              // char read

    while (count--) {
        while (tty_fifo_pull(&tty->fifo, &c) == FAILURE) {  // wait for a char from the keyboard
            thread_yield();                                 // nothing then we yield the processor
            irq_enable();                                   // get few characters if thread is alone
            irq_disable();                                  // close enter
        }
        *buf++ = c;
        res++;
    }
    return res;                                         // return the number of char read
}

static int soclib_tty_write(struct tty_s *tty, char *buf, unsigned count)
{
    int res = 0;                                        // nb of written char
    struct soclib_tty_regs_s *regs = 
        (struct soclib_tty_regs_s *) tty->address;      // access the registers
    
    while (count--) {                                   // while there are chars
        regs->write = *buf;                             // send the char to TTY
        delay(150);
        res++;                                          // nb of written char
        buf++;		                                    // but is the next address in buffer
    }
    return res;
}

struct tty_ops_s soclib_tty_ops = {
    .tty_init = soclib_tty_init,
    .tty_read = soclib_tty_read,
    .tty_write = soclib_tty_write
};

void soclib_tty_isr(unsigned irq, struct tty_s *tty)
{
    struct soclib_tty_regs_s *regs = 
        (struct soclib_tty_regs_s *) tty->address;
    char c = regs->read;
    tty_fifo_push(&tty->fifo, c);
}
