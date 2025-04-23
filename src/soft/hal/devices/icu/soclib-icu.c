/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     hal/devices/icu/soclib-icu.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Soclib ICU driver

\*------------------------------------------------------------------------------------------------*/

#include <hal/devices/icu/soclib-icu.h>

/**
 * Soclib device registers
 */

struct soclib_icu_regs_s {
    int state;          ///< state of all IRQ signals
    int mask;           ///< IRQ mask to chose what we need for this ICU
    int set;            ///< IRQ set   --> enable specific IRQs for this ICU
    int clear;          ///< IRQ clear --> disable specific IRQs for this ICU
    int highest;        ///< highest pritority IRQ number for this ICU
    int unused[3];      ///< 3 register addresses are not used
};

/**
 * \brief   Initialize the soclib icu device
 * \param   icu     icu device to initialize
 * \param   minor   minor number is the device instance number
 * \param   base    base of the physical device
 * \return  nothing
 */
static void soclib_icu_init (icu_t *icu, unsigned minor, unsigned base)
{
    icu->ops     = &SoclibICUOps;
    icu->minor   = minor;
    icu->base    = base;
}

/**
 * \brief     get the highest priority irq number for this ICU instance
 * \param     icu  icu the icu struct
 * \return    the highest priorty irq number that is for this ICU the lowest irq number
 */
static unsigned soclib_icu_get_highest (icu_t *icu)
{
    struct soclib_icu_regs_s *regs =
        (struct soclib_icu_regs_s *) icu->base;
    return regs->highest;
}

static void soclib_icu_set_priority (icu_t *icu, unsigned irq, unsigned pri)
{
    /* Not implemented */
    ;
}

static void soclib_icu_acknowledge (icu_t *icu, unsigned irq)
{
    /* Not implemented */
    ;
}

/**
 * \brief   Unmask the IRQ (enable it), (write 1 in the corresponding SET bit)
 * \param   icu icu device
 * \param   irq target irq
 * \return  nothing
 */
static void soclib_icu_unmask (icu_t *icu, unsigned irq)
{
    struct soclib_icu_regs_s *regs =
        (struct soclib_icu_regs_s *) icu->base;
    regs->set = 1 << irq;
}

static void soclib_icu_mask (icu_t *icu, unsigned irq)
{
    /* Not implemented */
    ;
}

struct icu_ops_s SoclibICUOps = {
    .icu_acknowledge    = soclib_icu_acknowledge,
    .icu_get_highest    = soclib_icu_get_highest,
    .icu_init           = soclib_icu_init,
    .icu_mask           = soclib_icu_mask,
    .icu_set_priority   = soclib_icu_set_priority,
    .icu_unmask         = soclib_icu_unmask
};

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
