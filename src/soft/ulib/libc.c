/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     ulib/libc.c
  \author   Franck Wajsburt
  \brief    IEEE lib.c subset

\*------------------------------------------------------------------------------------------------*/

#include <libc.h>       // external function declarations

void perror (char *s)
{
    if (s && *s)
        fprintf (0, "%s: %s\n", s, errno_mess[errno + 1]); // first number of errno is -1
    else
        fprintf (0, "%s\n", errno_mess[errno + 1]);
}

void exit (int status)
{
    syscall_fct( status, 0, 0, 0, SYSCALL_EXIT);        // never returns
}

void delay (unsigned nbcycles)
{
    unsigned old_time, expected_time;

    old_time = clock ();                        // get the number of cycles at first
    expected_time = old_time + nbcycles;        // compute the number of cycles to reach
    while (clock () < expected_time);           // wait for expected time
}

int read(int fd, void *buf, int count)
{
    return syscall_fct( fd, (int)buf, count, 0, SYSCALL_READ);
}

int write(int fd, void *buf, int count)
{
    return syscall_fct( fd, (int)buf, count, 0, SYSCALL_WRITE);
}

unsigned clock (void)
{
    return syscall_fct (0, 0, 0, 0, SYSCALL_CLOCK);
}

unsigned cpuid (void)
{
    return syscall_fct (0, 0, 0, 0, SYSCALL_CPUID);
}

void dcache_buf_inval (void *buf, size_t size)
{
    syscall_fct (0, 0, 0, 0, SYSCALL_DCACHEBUFINVAL);
}

void dcache_inval (void *addr)
{
    syscall_fct (0, 0, 0, 0, SYSCALL_DCACHEINVAL);
}

size_t cachelinesize (void)
{
    return syscall_fct (0, 0, 0, 0, SYSCALL_CACHELINESIZE);
}

static unsigned randnext = 1;

#if RAND_MAX > 0x7FFFFFFE
#   error k_libc.h: RAND_MAX has to be less than 0x7FFFFFFE
#endif

int rand (void)
{
    randnext = (randnext * 1103515245 + 12345) % ((unsigned) RAND_MAX + 1);
    return randnext;
}

void srand (unsigned seed)
{
    randnext = seed;
}

int fputc (int tty, int c)
{
    write (tty, &c, 1);                         // the char is in the least significant byte
    return c;
}

int fgetc (int tty)
{
    char buf[1];                                // create a buffer for the read char
    int res = read (tty, buf, 1);               // ask for one char from TTY
    if (res != 1) return 0;                     // if no char read, should return EOF
    res = write (tty, buf, 1);                  // send it back to the TTY
    if (res != 1) return 0;                     // if no char written, should return EOF
    return (int) buf[0];                        // return char read
}

int fgets (char *buf, int count, int tty)
{
    // to make the backspace, we use ANSI codes : https://www.wikiwand.com/en/ANSI_escape_code
    char *DEL = "\033[D \033[D";                // move left, then write ' ' and move left
    int res = 0;
    count--;                                    // we need to add a NUL (0) char at the end
    char c=0;

    while ((count != 0) && (c != '\n')) {       // as long as we can or need to get a char

        read (tty, &c, 1);                      // read only 1 char
        if (c == '\n')                          // if c is the line feed (10)
            read (tty, &c, 1);                  // drop it and read the next one
        if (c == '\r')                          // if c is the carriage return (13)
                c = '\n';                       // change it for '\n', that is what fgets waits for

        if ((c == 8)||(c == 127)) {             // 8 = backspace, 127 = delete
            if (res) {                          // go back in the buffer if possible
                write (tty, DEL, 7);            // erase current char
                count++;		                // count is the remaining place
                buf--;                          // but is the next address in buffer
                res--;
            }
            continue;                           // ask for another key
        } else {
            write (tty, &c, 1);                 // loop back to the tty
        }
        *buf = c;                               // write the char into the buffer
        buf++;                                  // increments the writing pointer
        count--;                                // decrements the remaining space
        res++;                                  // increments the read char
    }
    *buf = 0;                                   // add a last 0 to end the string

    return res;                                 // returns the number of char read
}

int fprintf(int tty, char *fmt, ...)
{
    static char buffer[PRINTF_MAX];
    int res;
    va_list ap;
    va_start (ap, fmt);
    res = vsnprintf(buffer, sizeof(buffer), fmt, ap);
    res = write( tty, buffer, res);
    va_end(ap);
    return res;
}

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
