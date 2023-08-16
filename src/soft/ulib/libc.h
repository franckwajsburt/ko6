/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-04
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     ulib/libc.h
  \author   Franck Wajsburt
  \brief    IEEE lib.c subset API
            - libc.h contains usual stdio.h, stdlib.h, string.h, etc.

\*------------------------------------------------------------------------------------------------*/

#ifndef _LIBC_H_
#define _LIBC_H_

#ifndef VERBOSE         // if verbose mode not already defined
#define VERBOSE 0       // set the default value
#endif
#if VERBOSE==1          // verbose mode
#  include <debug_on.h> // macro BIP, VAR, INFO and ASSERT will print something
#else
#  include <debug_off.h>// macro BIP, VAR, INFO and ASSERT are ignored
#endif

#ifndef __DEPEND__      // this condition allows to not include stdarg.h when makedepend is used
#include <stdarg.h>     // gcc's builtin include to use variadic function (https://bit.ly/3hLXjyC)
#include <stddef.h>     // gcc's builtin include with NULL, size_t, (https://bit.ly/3lBw3p6)
#endif
#include <cstd.h>       // generic C functions
#include <list.h>       // generic list management
#include <syscalls.h>   // kernel services
#include <memory.h>     // user allocator
#include <usermem.h>    // user data region usage
#include <errno.h>      // standard error code number
#include <esc_code.h>   // ANSI escape code

#define RAND_MAX 32767  /* maximum random value by default, must be < 0x7FFFFFFE */
#define PRINTF_MAX 256  /* largest printed message */
#define CEIL(a,b)       ((int)(b)*(((int)(a)+(int)(b)-1)/(int)(b))) /* round up a aligned on b */
#define FLOOR(a,b)      ((int)(b)*((int)(a)/(int)(b)))              /* round down a aligned on b */

#define EXIT(n)         *(char*)(0xFFFF0000+0x##n*16)=0 /* cause a panic with BAR = 0xFFFF-n-0 */

/**
 * \brief   finds the error message corresponding to the current value of the global variable 
 *          errno and writes it, followed by a newline (errno is thread safe)
 * \param   s is a string written if not NULL and not empty just before the system error
 */
extern void perror (char *s);

/**
 * \brief     exit the application, thus never returns
 * \param     status return value of the application
 */
extern void exit (int status);

/**
 * \brief     stop until a delay mesured in cycles
 * \param     nbcycles number of cycle to wait
 */
extern void delay (unsigned nbcycles);

/**
 * \brief     reads in at most count characters from fd and stores them into buf.
 * \param     fd    normally it is the file descriptor, but now it is just the tty number
 * \param     buf   pointer to the buffer
 * \param     count number of bytes to read, must be lower than buf size
 * \return    on success, the number of char read, else -1
 */
extern int read(int fd, void *buf, int count);

/**
 * \brief     write in at most count characters from buf to fd.
 * \param     fd    normally it is the file descriptor, but now it is just the tty number
 * \param     buf   pointer to the buffer
 * \param     count number of bytes to write, must be lower than buf size
 * \return    on success, the number of written char, else -1
 */
extern int write(int fd, void *buf, int count);

/**
 * \brief     clock cycle counter
 * \return    the number of cycles from the reset
 */
extern unsigned clock (void);

/**
 * \brief     ask the processor what is the L1-cache line size
 * \return    the cache line size in bytes
 */
extern size_t cachelinesize (void);

/**
 * \brief     invalidate all lines from data L1 cache belonging to buffer buf
 * \param     buf user buffer address
 * \param     size buffer size in bytes
 */
extern void dcache_buf_inval (void *buf, size_t size);

/**
 * \brief     invalidate the line from data L1 cache where the addr is located
 * \param     addr address into the line to invalidate
 */
extern void dcache_inval (void *addr);

/**
 * \brief     random int generator
 * \return    a random int between 0 and RAND_MAX
 * \details   https://en.wikipedia.org/wiki/Linear_congruential_generator (glibc generator)
 */
extern int rand (void);

/**
 * \brief     allow to define a specific random suite
 * \param     seed  any integer
 */
extern void srand (unsigned seed);

/**
 * \brief     write un single char on tty
 * \param     tty   tty number (between 0 and TTY_MAX-1)
 * \param     c     char to print
 * \return    the written char
 */
int fputc (int tty, int c);

/**
 * \brief     reads the next character from tty and returns it cast to an int
 * \param     tty   tty number (between 0 and TTY_MAX-1)
 */
extern int fgetc (int tty);

/**
 * \brief     reads in at most size-1 characters from tty and stores them into the buffer s.
 *            Reading stops after a newline. If a newline is read, it is stored into the buffer.
 *            A terminating null byte (0) is stored after the last character in the buffer.
 * \param     s     pointer to the buffer where the characters read will be written
 * \param     size  number of places in bytes in the buffer s.
 * \param     tty   tty number (between 0 and TTY_MAX-1) should be a FILE *
 */
extern int fgets (char *s, int size, int tty);

/**
 * \brief     this a simplified version which handles only: %c, %s, $d, %x and %p
 * \param     tty   tty number (between 0 and TTY_MAX-1)
 * \param     fmt   formated string
 * \param     ...   variadic arguments, i.e. variable number of arguments
 * \return    number of printed char
 */
extern int fprintf (int tty, char *fmt, ...);

#endif//_LIBC_H_
