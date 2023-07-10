/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-03
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/harch.h
  \author   Franck Wajsburt
  \brief    driver API for all devices which is implemented by harch.c

\*------------------------------------------------------------------------------------------------*/

#ifndef _HARCH_H_
#define _HARCH_H_

//--------------------------------------------------------------------------------------------------
// Architecture API
//--------------------------------------------------------------------------------------------------

/**
 * \brief     Read the device tree blob in memory and initialize
 *            every device on the SoC (ttys, block devices, ...)
*/
extern void soc_init ();

/**
 * \brief     Request the initialization of all the architecture
 *            This function configures each used devices
 *            and configures which IRQ are used and how they are connected to the CPU
 *            thanks to the ICU Mask
 * \param     tick is the time in cycles between to IRQ for the timer
 */
extern void arch_init (int tick);

//--------------------------------------------------------------------------------------------------
// DMA API
//--------------------------------------------------------------------------------------------------

/**
 * \brief     copies buffer src to the buffer dest with DMA device (blocking function)
 * \param     dest destination buffer
 * \param     src  source buffer
 * \param     n  number of integers to copy
 * \return    dest address on success or NULL 
 */
void * dma_memcpy (int *dest, int *src, unsigned n);

//--------------------------------------------------------------------------------------------------
// TTY API
//--------------------------------------------------------------------------------------------------

/**
 * \brief     read any char from the tty until count is reached
 * \param     tty   tty number (between 0 and NTTYS-1)
 * \param     buf   buffer where the read char are put
 * \param     count number of char read must be lower or equal to buf size
 * \return    the number of read chars
 */
int tty_read (int tty, char *buf, unsigned count);

/**
 * \brief     write count chars of the buffer to the tty
 * \param     tty   tty number (between 0 and NTTYS-1)
 * \param     buf   buffer pointer
 * \return    the number of written char
 */
int tty_write (int tty, char *buf, unsigned count);

#endif//_HARCH_H_
