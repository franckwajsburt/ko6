#include <hal/cpu/irq.h>

struct ite_s interrupt_table[MAX_N_IRQS];

void register_interrupt(unsigned irq, isr_t handler, void *arg)
{
    interrupt_table[irq].handler = handler;
    interrupt_table[irq].arg = arg;
}

void route_interrupt(unsigned irq)
{
    struct ite_s ite = interrupt_table[irq];
    if (ite.handler)
        ite.handler(irq, ite.arg);
}

void unregister_interrupt(unsigned irq)
{
    // TODO: register kpanic as a default isr, like the old system
    interrupt_table[irq].handler = (isr_t) 0;
    interrupt_table[irq].arg = (void*) 0;
}
