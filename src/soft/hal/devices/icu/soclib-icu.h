/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-14
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     hal/devices/icu/soclib-icu.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Soclib ICU driver

\*------------------------------------------------------------------------------------------------*/

#ifndef _SOCLIB_ICU_H_
#define _SOCLIB_ICU_H_

#include <hal/devices/icu.h>

/**
 * \brief See hal/device/icu.h for the function signature
 * .icu_init         : initialize the ICU device                           
 * .icu_get_highest  : fetch the highest priority IRQ from the ICU device
 * .icu_acknowledge  : should aknowledge an IRQ
 * .icu_mask         : mask (disable) an IRQ
 * .icu_unmask       : unmask (enable) an IRQ
 * .icu_set_priority : sets the priority of a given IRQ                    
 */
extern struct icu_ops_s SoclibICUOps;

#endif

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
