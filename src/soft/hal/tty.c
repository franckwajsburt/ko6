/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-10
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/tty.c
  \author   Franck Wajsburt, Nolan Bled
  \brief    Implementation of TTY helper functions (FIFO)

\*------------------------------------------------------------------------------------------------*/

#include <hal/tty.h>

/**
 * \brief   push a character into the tty's FIFO
 * \param   fifo    structure of fifo to store data
 * \param   c       char to write
 * \return  SUCCESS or FAILURE
 */
int tty_fifo_push(struct tty_fifo_s *fifo, char c)
{
    unsigned pt_write_next = (fifo->pt_write + 1) % sizeof(fifo->data);
    if (pt_write_next != fifo->pt_read) {
        fifo->data [fifo->pt_write] = c;
        fifo->pt_write = pt_write_next;
        return SUCCESS;   
    }
    return FAILURE;
}

/**
 * \brief   pop a character from the tty's FIFO
 * \param   fifo    structure of fifo to store data
 * \param   c       pointer on char to put the read char 
 * \return  SUCCESS or FAILURE
 */
int tty_fifo_pull (struct tty_fifo_s *fifo, int *c)
{
    if (fifo->pt_read != fifo->pt_write) {
        *c = fifo->data [fifo->pt_read];
        fifo->pt_read = (fifo->pt_read + 1)% sizeof(fifo->data);
        return SUCCESS;
    }
    return FAILURE;
}