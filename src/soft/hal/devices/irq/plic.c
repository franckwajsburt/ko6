/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-08-01
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/devices/irq/plic.c
  \author   Nolan Bled
  \brief    PLIC driver 

\*------------------------------------------------------------------------------------------------*/

#include <hal/devices/irq/plic.h>

/**
 * \brief   Initialize PLIC device
 * \param   irq     irq device to initialize
 * \param   base    The base address of the physical device
 * \return  nothing
 */
static void plic_init(struct irq_s *irq, unsigned base)
{
    irq->ops     = &PlicOps;
    irq->base    = base;
}

/**
 * \brief   Get the highest priority IRQ triggered (mclaim register of the PLIC)*
 * \param   irq irq device
 */
static unsigned plic_get_highest(struct irq_s *irq)
{
    return *(unsigned*) PLIC_MCLAIM(irq->base, cpuid());
}

/**
 * \brief   Set IRQ priority
 * \param   irq irq device
 * \param   irq target irq
 * \param   pri the new irq priority
 * \return  nothing
 */
static void plic_set_priority(struct irq_s *irq, unsigned irq, unsigned pri)
{
    ((unsigned*)irq->base + PLIC_PRI_OFFSET)[irq] = pri;
}

/**
 * \brief   Acknowledge the IRQ (write the IRQ no in the MCLAIM register)
 * \param   irq irq device
 * \param   irq target irq
 * \return  nothing
 */
static void plic_acknowledge(struct irq_s *irq, unsigned irq)
{
    *(unsigned*) PLIC_MCLAIM(irq->base, cpuid()) = irq;
}

/**
 * \brief   Unmask the IRQ (enable it), (write 1 in the corresponding MENABLE bit)
 * \param   irq irq device
 * \param   irq target irq
 * \return  nothing
 */
static void plic_unmask(struct irq_s *irq, unsigned irq)
{
    unsigned j = irq % 32;
    *(unsigned*) PLIC_MENABLE(irq->base, cpuid(), irq) |= (1 << j);
}

/**
 * \brief   Mask the IRQ (disable it), (write 0 in the corresponding MENABLE bit)
 * \param   irq irq device
 * \param   irq target irq
 * \return  nothing
 */
static void plic_mask(struct irq_s *irq, unsigned irq)
{
    unsigned j = irq % 32;
    *(unsigned*) PLIC_MENABLE(irq->base, cpuid(), irq) &= ~(1 << j);
}

struct irq_ops_s PlicOps = {
    .irq_acknowledge = plic_acknowledge,
    .irq_get_highest = plic_get_highest,
    .irq_init = plic_init,
    .irq_mask = plic_mask,
    .irq_set_priority = plic_set_priority,
    .irq_unmask = plic_unmask
};
