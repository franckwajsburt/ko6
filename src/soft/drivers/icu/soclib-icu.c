#include <drivers/icu/soclib-icu.h>

void soclib_icu_init(struct icu_s *icu, unsigned address)
{
    icu->ops        = &soclib_icu_ops;
    icu->address    = address;
}

/**
 * \brief     get the highest priority irq number for an ICU instance
 *            There are as many ICU as CPU.
 *            Each ICU allows to select which IRQ must be handled by the corresponding CPU
 *            The IRQ selection is done thanks to the icu_set_mask() functioni.
 *            If there are several IRQs active at the same time, the register named "highest"
 *            gives the IRQ number with the highest priority
 * \param     icu  icu the icu struct
 * \return    the highest priorty irq number that is for this ICU the lowest irq number
 */
unsigned soclib_icu_get_highest(struct icu_s *icu)
{
    struct soclib_icu_regs_s *regs =
        (struct soclib_icu_regs_s *) icu->address;
    return regs->highest;
}

void soclib_icu_set_priority(struct icu_s *icu, unsigned irq, unsigned pri)
{
    /* Not implemented */
    ;
}

void soclib_icu_acknowledge(struct icu_s *icu, unsigned irq)
{
    /* Not implemented */
    ;
}

/**
 * \brief     enable device IRQ siqnal
 * \param     icu  icu
 * \param     irq  irq number to enable
 */
void soclib_icu_unmask(struct icu_s *icu, unsigned irq)
{
    struct soclib_icu_regs_s *regs =
        (struct soclib_icu_regs_s *) icu->address;
    regs->set = 1 << irq;
}

void soclib_icu_mask(struct icu_s *icu, unsigned irq)
{
    /* Not implemented */
    ;
}

struct icu_ops_s soclib_icu_ops = {
    .icu_acknowledge = soclib_icu_acknowledge,
    .icu_get_highest = soclib_icu_get_highest,
    .icu_init = soclib_icu_init,
    .icu_mask = soclib_icu_mask,
    .icu_set_priority = soclib_icu_set_priority,
    .icu_unmask = soclib_icu_unmask
};