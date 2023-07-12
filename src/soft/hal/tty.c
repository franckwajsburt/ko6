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

/* Helper functions to register and access TTYs per number */

struct tty_s* tty_get(unsigned no)
{
    list_foreach(&ttyList, item) {
        struct tty_s *tty = list_item(item, struct tty_s, list);
        if (tty->no == no)
            return tty;
    }
    return NULL;
}

unsigned tty_add(struct tty_s *tty)
{
    struct list_s *last = list_last(&ttyList);
    if (list_last(&ttyList) == &ttyList)
        tty->no = 0;
    else
        tty->no = list_item(last, struct tty_s, list)->no + 1;
    list_addlast(&ttyList, &tty->list);
    return tty->no;
}

void tty_del(unsigned no)
{
    struct tty_s *tty = tty_get(no);
    if (tty)
        list_unlink(&tty->list);
}