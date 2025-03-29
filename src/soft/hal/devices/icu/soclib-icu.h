/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-08-01
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/devices/icu/soclib-icu.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Soclib ICU driver

\*------------------------------------------------------------------------------------------------*/

#ifndef _SOCLIB_ICU_H_
#define _SOCLIB_ICU_H_

#include <hal/devices/icu.h>

struct soclib_icu_regs_s {
    int state;          ///< state of all IRQ signals
    int mask;           ///< IRQ mask to chose what we need for this ICU
    int set;            ///< IRQ set   --> enable specific IRQs for this ICU
    int clear;          ///< IRQ clear --> disable specific IRQs for this ICU
    int highest;        ///< highest pritority IRQ number for this ICU
    int unused[3];      ///< 3 register addresses are not used
};

extern struct icu_ops_s SoclibICUOps;

#endif
