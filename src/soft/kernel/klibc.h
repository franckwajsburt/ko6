/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     kernel/klib.h
  \author   Franck Wajsburt
  \brief    Standard general purpose functions API for the kernel
            - This file include all other kernel include files in order to simplify 
              the file to include in a c file.
            - we just need to include <klibc.h> in most of c file

\*------------------------------------------------------------------------------------------------*/

#ifndef _KLIBC_H_
#define _KLIBC_H_

#ifndef _KERNEL_
#define _KERNEL_                    // This include are dedicated to kernel
#endif

#ifndef VERBOSE                     // if verbose mode not already defined
#define VERBOSE 0                   // set the default value
#endif
#if VERBOSE==1
#  include <common/debug_on.h>      // macro BIP, VAR, INFO and ASSERT will print something
#else
#  include <common/debug_off.h>     // macro BIP, VAR, INFO and ASSERT are ignored
#endif

#ifndef __DEPEND__      // this condition allows to not include stdarg.h when makedepend is used
#include <stdarg.h>     // gcc's builtin include to use variadic function (https://bit.ly/3hLXjyC)
#include <stddef.h>     // gcc's builtin include with NULL, size_t, (https://bit.ly/3lBw3p6)
#endif//__DEPEND__

#include <common/esc_code.h>        // ANSI escape code
#include <common/cstd.h>            // generic C functions
#include <common/list.h>            // generic list management
#include <common/errno.h>           // standard error code number
#include <common/syscalls.h>        // syscall's codes
#include <common/kshell_syscalls.h> // kshell syscall's codes
#include <common/usermem.h>         // user data region usage
#include <common/htopen.h>          // hash table open addressing
#include <common/ctype.h>           // ascii types

#include <fs/pvfs.h>                // pseudo vitual file system

#include <hal/devices/chardev.h>

#include <hal/cpu/atomic.h>         // Locks
#include <hal/cpu/cache.h>          // L1 cache function prototypes
#include <hal/cpu/thread.h>         // threads context
#include <hal/cpu/irq.h>            // 
#include <hal/cpu/cpuregs.h>        // CPU registers manipulation function prototypes
#include <hal/cpu/kpanic.h>

#include <hal/devices/blockdev.h>   // block devices
#include <hal/devices/chardev.h>    // char devices 
#include <hal/devices/dma.h>        // direct memory access
#include <hal/devices/icu.h>        // interrupt component unit
#include <hal/devices/timer.h>      // timers
#include <kernel/kirq.h>            // irq registering
#include <kernel/kdev.h>            // generic devive management

#include <hal/soc/soc.h>

#include <kernel/kmemkernel.h>      // kernel allocators
#include <kernel/kmemuser.h>        // kernel part of user allocators 
#include <kernel/kthread.h>         // thread functions and scheduler
#include <kernel/ksynchro.h>        // mutex, barrier and similar functions
#include <kernel/kdev.h>            // dynamic devices allocation
#include <kernel/kshell.h>          // kshell syscall
#include <kernel/kblockio.h>        // block device's request queue and buffer cache

#define RAND_MAX 32767  /* maximum random value by default, must be < 0x7FFFFFFE */
#define PRINTF_MAX 512  /* largest printed message */
#define CEIL(a,b)       ((int)(b)*(((int)(a)+(int)(b)-1)/(int)(b))) /* round up a aligned on b */
#define FLOOR(a,b)      ((int)(b)*((int)(a)/(int)(b)))              /* round down a aligned on b */
#define FIFO_DEPTH 20   /* maximum fifo depth */

#define V(fmt,v) kprintf("%s : "fmt, #v, (v))

#define O_FILE          __usermem.o_file

//--------------------------------------------------------------------------------------------------

/**
 * \brief     random int generator specificaly for the kernel usage
 * \return    a random int between 0 and RAND_MAX
 */
extern int krand (void);

/**
 * \brief     random int generator FIXME must be replaced by /dev/urandom
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
 * \brief Wrapper function that selects the correct TTY based on his number
 *        and call the read function
 * \param tty   the TTY's number
 * \param buf   target buffer where the user's entry will be typed
 * \param count number of bytes to write into buffer
 */
extern int tty_read (int tty, char *buf, unsigned count);

/**
 * \brief Wrapper function that selects the correct TTY based on his number
 *        and call the write function
 * \param tty   the TTY's number
 * \param buf   target buffer sent to the tty
 * \param count number of bytes to read into buffer
 */
extern int tty_write (int tty, char *buf, unsigned count);

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

/**
 * \brief Simple fifo (1 writer - 1 reader)
 *          - data      buffer of data
 *          - pt_write  write pointer for L fifos (0 at the beginning)
 *          - pt_read   read pointer for L fifos (0 at the beginning)
 *
 *        data[] is used as a circular array. At the beginning (pt_read == pt_write) means an empty fifo
 *        then when we push a data, we write it at pt_write, the we increment pt_write % fifo_size.
 *        The fifo is full when it remains only one free cell, then when (pt_write + 1)%size == pt_read
 */
struct fifo_s {
    char data [FIFO_DEPTH];         ///< Circular array
    unsigned pt_read;               ///< Read pointer
    unsigned pt_write;              ///< Write pointer
};

/* Helper functions for CHARDEV's FIFOs */

/**
 * \brief   push a character into the chardev's FIFO
 * \param   fifo    structure of fifo to store data
 * \param   c       char to write
 * \return  SUCCESS or FAILURE
 */
extern int fifo_push (struct fifo_s *fifo, char c);

/**
 * \brief   pop a character from the chardev's FIFO
 * \param   fifo    structure of fifo to store data
 * \param   c       pointer on char to put the read char 
 * \return  SUCCESS or FAILURE
 */
extern int fifo_pull (struct fifo_s *fifo, char *c);

void tick_event (void);
#endif//_KLIBC_H_

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
