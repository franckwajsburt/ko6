#include <drivers/icu/plic.h>

void plic_init(struct icu_s *icu, unsigned address)
{
    icu->ops        = &plic_ops;
    icu->address    = address;
}

unsigned plic_get_highest(struct icu_s *icu)
{
    return *(unsigned*) PLIC_MCLAIM(icu->address, cpuid());
}

void plic_set_priority(struct icu_s *icu, unsigned irq, unsigned pri)
{
    ((unsigned*)icu->address + PLIC_PRI_OFFSET)[irq] = pri;
}

void plic_acknowledge(struct icu_s *icu, unsigned irq)
{
    *(unsigned*) PLIC_MCLAIM(icu->address, cpuid()) = irq;
}

void plic_unmask(struct icu_s *icu, unsigned irq)
{
    unsigned j = irq % 32;
    *(unsigned*) PLIC_MENABLE(icu->address, cpuid(), irq) |= (1 << j);
}

void plic_mask(struct icu_s *icu, unsigned irq)
{
    unsigned j = irq % 32;
    *(unsigned*) PLIC_MENABLE(icu->address, cpuid(), irq) &= ~(1 << j);
}

struct icu_ops_s plic_ops = {
    .icu_acknowledge = plic_acknowledge,
    .icu_get_highest = plic_get_highest,
    .icu_init = plic_init,
    .icu_mask = plic_mask,
    .icu_set_priority = plic_set_priority,
    .icu_unmask = plic_unmask
};