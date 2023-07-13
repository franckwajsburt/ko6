/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-12
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/tty.c
  \author   Franck Wajsburt, Nolan Bled
  \brief    TTYs management helper functions

\*------------------------------------------------------------------------------------------------*/

#include <hal/tty.h>

list_t ttyList = {
    .next = &ttyList,
    .prev = &ttyList
};

int tty_fifo_push (struct tty_fifo_s *fifo, char c)
{
    unsigned pt_write_next = (fifo->pt_write + 1) % sizeof(fifo->data);
    if (pt_write_next != fifo->pt_read) {
        fifo->data [fifo->pt_write] = c;
        fifo->pt_write = pt_write_next;
        return SUCCESS;   
    }
    return FAILURE;
}

int tty_fifo_pull (struct tty_fifo_s *fifo, int *c)
{
    if (fifo->pt_read != fifo->pt_write) {
        *c = fifo->data [fifo->pt_read];
        fifo->pt_read = (fifo->pt_read + 1)% sizeof(fifo->data);
        return SUCCESS;
    }
    return FAILURE;
}