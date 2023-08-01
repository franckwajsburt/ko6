#include <hal/cpu/irq.h>

struct ite_s InterruptTable[MAX_N_IRQS];

void register_interrupt(unsigned irq, isr_t handler, void *arg)
{
    InterruptTable[irq].handler = handler;
    InterruptTable[irq].arg = arg;
}

void route_interrupt(unsigned irq)
{
    struct ite_s ite = InterruptTable[irq];
    if (ite.handler)
        ite.handler(irq, ite.arg);
}

void unregister_interrupt(unsigned irq)
{
    // TODO: register kpanic as a default isr, like the old system
    InterruptTable[irq].handler = (isr_t) 0;
    InterruptTable[irq].arg = (void*) 0;
}
