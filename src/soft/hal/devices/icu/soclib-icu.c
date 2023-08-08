/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-08-01
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/devices/icu/soclib-icu.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Soclib ICU driver

\*------------------------------------------------------------------------------------------------*/

#include <hal/devices/icu/soclib-icu.h>

/**
 * \brief   Initialize the soclib icu device
 * \param   icu icu device to initialize
 * \param   address address of the physical device
 * \return  nothing
 */
static void soclib_icu_init(struct icu_s *icu, unsigned address)
{
    icu->ops        = &SoclibICUOps;
    icu->address    = address;
}

/**
 * \brief     get the highest priority irq number for this ICU instance
 * \param     icu  icu the icu struct
 * \return    the highest priorty irq number that is for this ICU the lowest irq number
 */
static unsigned soclib_icu_get_highest(struct icu_s *icu)
{
    struct soclib_icu_regs_s *regs =
        (struct soclib_icu_regs_s *) icu->address;
    return regs->highest;
}

static void soclib_icu_set_priority(struct icu_s *icu, unsigned irq, unsigned pri)
{
    /* Not implemented */
    ;
}

static void soclib_icu_acknowledge(struct icu_s *icu, unsigned irq)
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
static void soclib_icu_unmask(struct icu_s *icu, unsigned irq)
{
    struct soclib_icu_regs_s *regs =
        (struct soclib_icu_regs_s *) icu->address;
    regs->set = 1 << irq;
}

static void soclib_icu_mask(struct icu_s *icu, unsigned irq)
{
    /* Not implemented */
    ;
}

struct icu_ops_s SoclibICUOps = {
    .icu_acknowledge = soclib_icu_acknowledge,
    .icu_get_highest = soclib_icu_get_highest,
    .icu_init = soclib_icu_init,
    .icu_mask = soclib_icu_mask,
    .icu_set_priority = soclib_icu_set_priority,
    .icu_unmask = soclib_icu_unmask
};