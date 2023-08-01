/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-11
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/cpu/cpu.h
  \author   Franck Wajsburt
  \brief    Generic CPU functions prototypes

\*------------------------------------------------------------------------------------------------*/

#ifndef _HAL_CPU_H_
#define _HAL_CPU_H_

//--------------------------------------------------------------------------------------------------
// Special registers
//--------------------------------------------------------------------------------------------------

/**
 * \brief     clock cycle counter
 * \return    the number of cycles from the reset
 */
extern unsigned clock (void);

/** \brief  cpu identifier
 *  \return the current cpu identifier from 0 to NCPUS-1
 */
extern unsigned cpuid (void);

/**
 * \brief  prints all registers' value on TTY0 (must be in kernel mode) then stops program
 */
extern void kpanic (void);

#endif
