/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-04-04
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     kernel/klib.h
  \author   Franck Wajsburt
  \brief    Standard general purpose functions API for the kernel
            - This file include all other kernel include files in order to simplify 
              the file to include in a c file.
            - we just need to include <klibc.h> in most of c file

\*------------------------------------------------------------------------------------------------*/

#ifndef _KLIBC_H_
#define _KLIBC_H_

#define _KERNEL_        // to tell, it is kernel code, usefull for debug PRINT

#ifndef VERBOSE         // if verbose mode not already defined
#define VERBOSE 0       // set the default value
#endif
#if VERBOSE==1
#  include <debug_on.h> // macro BIP, VAR, INFO and ASSERT will print something
#else
#  include <debug_off.h>// macro BIP, VAR, INFO and ASSERT are ignored
#endif

#ifndef __DEPEND__      // this condition allows to not include stdarg.h when makedepend is used
#include <stdarg.h>     // gcc's builtin include to use variadic function (https://bit.ly/3hLXjyC)
#include <stddef.h>     // gcc's builtin include with NULL, size_t, (https://bit.ly/3lBw3p6)
#endif//__DEPEND__
#include <cstd.h>       // generic C functions
#include <list.h>       // generic list management
#include <hcpu.h>       // CPU-dependent function prototypes
#include <harch.h>      // functions ARCH dependant
#include <usermem.h>    // user data region usage
#include <kmemory.h>    // all kernel allocators
#include <kthread.h>    // thread functions
#include <esc_code.h>   // ANSI escape code
#include <errno.h>      // standard error code number
#include <syscalls.h>   // syscall's codes
#include <ksynchro.h>   // mutex, barrier and similar functions

#define RAND_MAX 32767  /* maximum random value by default, must be < 0x7FFFFFFE */
#define PRINTF_MAX 256  /* largest printed message */
#define CEIL(a,b)       ((int)(b)*(((int)(a)+(int)(b)-1)/(int)(b))) /* round up a aligned on b */
#define FLOOR(a,b)      ((int)(b)*((int)(a)/(int)(b)))              /* round down a aligned on b */

//--------------------------------------------------------------------------------------------------

/**
 * \brief     random int generator
 *            https://en.wikipedia.org/wiki/Linear_congruential_generator (glibc generator)
 * \return    a random int between 0 and RAND_MAX
 */
extern int rand (void);

/**
 * \brief     allow to define a specific random suite
 * \param     seed  any integer
 */
extern void srand (unsigned seed);

/**
 * \brief     stop until a delay mesured in cycles
 * \param     nbcycles number of cycle to wait
 */
extern void delay (unsigned nbcycles);

/**
 * \brief     print a formated string to the TTY0
 *            this a simplified version which handles only: %c, %s, $d, %x and %p
 * \param     fmt   formated string
 * \param     ...   variadic arguments, i.e. variable number of arguments
 * \return    number of printed char
 */
extern int kprintf (char *fmt, ...);

/**
 * \brief     exit the application, thus never returns
 * \param     status return value of the application
 */
extern void exit (int status);

/**
 * \brief     write a single char to the tty
 * \param     tty   tty number (between 0 and NTTYS-1)
 * \param     c     the char to write
 * \return    the written character
 */
extern int tty_putc (int tty, int c);

/**
 * \brief     read a single char from the tty
 * \param     tty   tty number (between 0 and NTTYS-1)
 * \return    the read char
 */
extern int tty_getc (int tty);

/**
 * \brief     send all char of a buffer to the tty until a 0 is found
 * \param     tty   tty number (between 0 and NTTYS-1)
 * \param     buf   buffer pointer
 * \return    the number of written char
 */
extern int tty_puts (int tty, char *buf);

/**
 * \brief     read any char from the tty until \n is found or count-1 is reached, add 0 at the end
 * \param     tty   tty number (between 0 and NTTYS-1)
 * \param     buf   buffer where the read char are put
 * \param     count buffer size
 * \return    the number of read chars
 */
extern int tty_gets (int tty, char *buf, int count);

#endif//_KLIBC_H_
