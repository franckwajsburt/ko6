/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     hal/devices/icu/plic.c
  \author   Nolan Bled
  \brief    PLIC driver 

\*------------------------------------------------------------------------------------------------*/

#include <hal/devices/icu/plic.h>

/**
 * \brief   Initialize PLIC device
 * \param   icu   icu device to initialize
 * \param   minor minor number is the device instance number
 * \param   base  The base address of the physical device
 * \return  nothing
 */
static void plic_init (icu_t *icu, unsigned minor, unsigned base)
{
    icu->ops    = &PlicOps;
    icu->minor  = minor;
    icu->base   = base;
}

/**
 * \brief   Get the highest priority IRQ triggered (mclaim register of the PLIC)*
 * \param   icu icu device
 */
static unsigned plic_get_highest (icu_t *icu)
{
    return *(unsigned*) PLIC_MCLAIM(icu->base, cpuid ());
}

/**
 * \brief   Set IRQ priority
 * \param   icu icu device
 * \param   irq target irq
 * \param   pri the new irq priority
 * \return  nothing
 */
static void plic_set_priority (icu_t *icu, unsigned irq, unsigned pri)
{
    ((unsigned*)icu->base + PLIC_PRI_OFFSET)[irq] = pri;
}

/**
 * \brief   Acknowledge the IRQ (write the IRQ no in the MCLAIM register)
 * \param   icu icu device
 * \param   irq target irq
 * \return  nothing
 */
static void plic_acknowledge (icu_t *icu, unsigned irq)
{
    *(unsigned*) PLIC_MCLAIM(icu->base, cpuid()) = irq;
}

/**
 * \brief   Unmask the IRQ (enable it), (write 1 in the corresponding MENABLE bit)
 * \param   icu icu device
 * \param   irq target irq
 * \return  nothing
 */
static void plic_unmask (icu_t *icu, unsigned irq)
{
    unsigned j = irq % 32;
    *(unsigned*) PLIC_MENABLE(icu->base, cpuid (), irq) |= (1 << j);
}

/**
 * \brief   Mask the IRQ (disable it), (write 0 in the corresponding MENABLE bit)
 * \param   icu icu device
 * \param   irq target irq
 * \return  nothing
 */
static void plic_mask (icu_t *icu, unsigned irq)
{
    unsigned j = irq % 32;
    *(unsigned*) PLIC_MENABLE(icu->base, cpuid(), irq) &= ~(1 << j);
}

struct icu_ops_s PlicOps = {
    .icu_acknowledge = plic_acknowledge,
    .icu_get_highest = plic_get_highest,
    .icu_init = plic_init,
    .icu_mask = plic_mask,
    .icu_set_priority = plic_set_priority,
    .icu_unmask = plic_unmask
};
