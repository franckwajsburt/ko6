/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     kernel/klib.c
  \author   Franck Wajsburt
  \brief    Standard general purpose functions for the kernel

\*------------------------------------------------------------------------------------------------*/

#include <kernel/klibc.h>      // external function declarations

#define randseed   *(thread_krandseed(ThreadCurrent))   /* Thread Local Storage for the kernel */

static int Krandseed;

int krand (void)                                // www.wikiwand.com/en/Linear_congruential_generator
{
    Krandseed = Krandseed * 1664525 + 1013904223; // Numerical Recipes
    return (Krandseed>>1);
}

int rand (void)                                 // www.wikiwand.com/en/Linear_congruential_generator
{
    randseed = randseed * 1664525 + 1013904223; // Numerical Recipes
    return (randseed>>1);
}

void srand (unsigned seed)
{
    randseed = seed;
}

void delay (unsigned nbcycles)
{
    unsigned old_time, expected_time;

    old_time = clock ();                        // get the number od cycles at first
    expected_time = old_time + nbcycles;        // compute the number of cycles to reach
    while (clock () < expected_time);           // wait for expected time
}

int kprintf(char *fmt, ...)
{
    static char buffer[PRINTF_MAX];
    va_list ap;
    va_start (ap, fmt);
    int res = vsnprintf(buffer, sizeof(buffer), fmt, ap);
    tty_write(0, buffer, res);
    va_end(ap);
    return res;
}

void exit (int status)
{
    PANIC_IF(true,"\n\nEXIT status = %d\n", status); 
}

int tty_read (int tty, char *buf, unsigned count)
{
    /* If the tty is not available, default to 0 */
    if (tty > chardev_count())
        tty = 0;

    struct chardev_s *cdev = chardev_get(tty);
    if (cdev)
        return cdev->ops->chardev_read(cdev, buf, count);
    // TODO: do a more specific error if the TTY is unavailable
    return -1;
}

int tty_write (int tty, char *buf, unsigned count)
{
    /* If the tty is not available, default to 0 */
    if (tty > chardev_count())
        tty = 0;
        
    struct chardev_s *cdev = chardev_get(tty);
    if (cdev)
        return cdev->ops->chardev_write(cdev, buf, count);
    return -1;
}

int tty_putc (int tty, int c)
{
    tty_write (tty, (char *)&c, 1);             // only write one char
    return c;
}

int tty_getc (int tty)
{
    char c;
    tty_read (tty, &c, 1);                      // read only 1 char
    tty_write (tty, &c, 1);                     // loop back to the tty
    return c;                                   // return the read char
}

int tty_puts (int tty, char *buf)
{
    int count = 0;                              // nb of written char
    while (buf[count++]);                       // count the number of char
    return tty_write (tty, buf, count);         // send buf to the tty
}

int tty_gets (int tty, char *buf, int count)
{
    // to make the backspace, we use ANSI codes : https://www.wikiwand.com/en/ANSI_escape_code
    char *DEL = "\033[D \033[D";                // move left, then write ' ' and move left
    int res = 0;
    count--;                                    // we need to add a NUL (0) char at the end
    char c=0;

    while ((count != 0) && (c != '\n')) {       // as long as we can or need to get a char

        tty_read (tty, &c, 1);                  // read only 1 char
        if (c == '\r')                          // if c is the carriage return (13)
            tty_read (tty, &c, 1);              // get the next that is line feed (10)

        if ((c == 8)||(c == 127)) {             // 8 = backspace, 127 = delete
            if (res) {                          // go back in the buffer if possible
        	    tty_write (tty, DEL, 7);        // replace the current char by a ' ' (space)
                count++;		                // count is the remaining place
                buf--;                          // but is the next address in buffer
                res--;
            }
            continue;                           // ask for another key
        } else
            tty_write (tty, &c, 1);             // loop back to the tty

        *buf = c;                               // write the char into the buffer
        buf++;                                  // increments the writing pointer
        count--;                                // decrements the remaining space
        res++;                                  // increments the read char
    }
    *buf = 0;                                   // add a last 0 to end the string

    return res;                                 // returns the number of char read
}

int fifo_push (struct fifo_s *fifo, char c)
{
    unsigned pt_write_next = (fifo->pt_write + 1) % sizeof(fifo->data);
    if (pt_write_next != fifo->pt_read) {
        fifo->data [fifo->pt_write] = c;
        fifo->pt_write = pt_write_next;
        return SUCCESS;
    }
    return FAILURE;
}

int fifo_pull (struct fifo_s *fifo, char *c)
{
    if (fifo->pt_read != fifo->pt_write) {
        *c = fifo->data [fifo->pt_read];
        fifo->pt_read = (fifo->pt_read + 1)% sizeof(fifo->data);
        return SUCCESS;
    }
    return FAILURE;
}

//--------------------------------------------------------------------------------------------------
// kcmd 
//--------------------------------------------------------------------------------------------------

//#define KCMD // Comment that to avoid ko6 kernel command

#ifndef KCMD
#define kcmd(tty)
#else
void kcmd (int tty) 
{
    static int  start;

    static char prompt[] = "\nko6> ";
    static char delete[] = "\b \b";
    static char cr[] = "\n";
    static char buffer[256];
    static int  count;
    static ht_t * ht ;

    char c;
    long val;
    unsigned try;

    if (start == 0) {
        tty_write (tty, prompt, sizeof (prompt));
        ht = ht_create (128);
        start = 1;
    }
    if (tty_read (0, &c, 0) == SUCCESS) {
        switch (c) {
        case 127 :  
            if (count) {
                count--;
                tty_write (tty, delete, sizeof (delete)); 
            }
            break;
        case '\n':
            buffer [count] = '\0';
            tty_write (tty, cr, sizeof (cr)); 
            if ((val = (long)ht_get (ht, buffer))) {        // ht_get return NULL at first
                try = ht_set (ht, buffer, (void *)(++val)); // if not increment the value
            } else {
                try = ht_set (ht, buffer, (void *)1); // set a new val
                val = 1;
            }
            PANIC_IF(try == -1, "kshell hash table too small");
            count = 0;
            kprintf ("%s = %d", buffer, val);
            tty_write (tty, prompt, sizeof (prompt));
//            ht_stat(ht);
            break;
        default:
            if (isprint(c) && (count < (sizeof (buffer) - 1))) {
                buffer [count++] = c;
                tty_write (tty, &c, 1); 
            }
       } 
   } 
}
#endif

void tick_event (void)
{
    thread_yield();
    kcmd(0);
}

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
