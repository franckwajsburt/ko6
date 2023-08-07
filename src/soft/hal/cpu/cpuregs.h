/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-11
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/cpu/cpuregs.h
  \author   Franck Wajsburt
  \brief    Access to CPU-specific registers

\*------------------------------------------------------------------------------------------------*/

#ifndef _HAL_CPUREGS_H_
#define _HAL_CPUREGS_H_

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

#endif
