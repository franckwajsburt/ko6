/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-02
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/cpu/kpanic.h
  \author   Franck Wajsburt
  \brief    declaration of kpanic()

\*------------------------------------------------------------------------------------------------*/

#ifndef _HAL_CPU_KPANIC_H_
#define _HAL_CPU_KPANIC_H_

/**
 * \brief  prints all registers' value on TTY0 (must be in kernel mode) then stops program
 */
extern void kpanic (void);

#endif