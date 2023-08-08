/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-08-01
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/devices/icu/plic.c
  \author   Nolan Bled
  \brief    PLIC driver 

\*------------------------------------------------------------------------------------------------*/

#include <hal/devices/icu/plic.h>

/**
 * \brief   Initialize PLIC device
 * \param   icu icu device to initialize
 * \param   address address of the physical device
 * \return  nothing
 */
static void plic_init(struct icu_s *icu, unsigned address)
{
    icu->ops        = &PlicOps;
    icu->address    = address;
}

/**
 * \brief   Get the highest priority IRQ triggered (mclaim register of the PLIC)*
 * \param   icu icu device
 */
static unsigned plic_get_highest(struct icu_s *icu)
{
    return *(unsigned*) PLIC_MCLAIM(icu->address, cpuid());
}

/**
 * \brief   Set IRQ priority
 * \param   icu icu device
 * \param   irq target irq
 * \param   pri the new irq priority
 * \return  nothing
 */
static void plic_set_priority(struct icu_s *icu, unsigned irq, unsigned pri)
{
    ((unsigned*)icu->address + PLIC_PRI_OFFSET)[irq] = pri;
}

/**
 * \brief   Acknowledge the IRQ (write the IRQ no in the MCLAIM register)
 * \param   icu icu device
 * \param   irq target irq
 * \return  nothing
 */
static void plic_acknowledge(struct icu_s *icu, unsigned irq)
{
    *(unsigned*) PLIC_MCLAIM(icu->address, cpuid()) = irq;
}

/**
 * \brief   Unmask the IRQ (enable it), (write 1 in the corresponding MENABLE bit)
 * \param   icu icu device
 * \param   irq target irq
 * \return  nothing
 */
static void plic_unmask(struct icu_s *icu, unsigned irq)
{
    unsigned j = irq % 32;
    *(unsigned*) PLIC_MENABLE(icu->address, cpuid(), irq) |= (1 << j);
}

/**
 * \brief   Mask the IRQ (disable it), (write 0 in the corresponding MENABLE bit)
 * \param   icu icu device
 * \param   irq target irq
 * \return  nothing
 */
static void plic_mask(struct icu_s *icu, unsigned irq)
{
    unsigned j = irq % 32;
    *(unsigned*) PLIC_MENABLE(icu->address, cpuid(), irq) &= ~(1 << j);
}

struct icu_ops_s PlicOps = {
    .icu_acknowledge = plic_acknowledge,
    .icu_get_highest = plic_get_highest,
    .icu_init = plic_init,
    .icu_mask = plic_mask,
    .icu_set_priority = plic_set_priority,
    .icu_unmask = plic_unmask
};