/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-08-07
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     kernel/kirq.c
  \author   Franck Wajsburt, Nolan Bled
  \brief    IRQs functions

\*------------------------------------------------------------------------------------------------*/

#include <kernel/kirq.h>

static struct ite_s InterruptVector[MAX_N_IRQS];

void register_interrupt(unsigned irq, isr_t handler, void *arg)
{
    InterruptVector[irq].handler = handler;
    InterruptVector[irq].arg = arg;
}

void route_interrupt(unsigned irq)
{
    struct ite_s ite = InterruptVector[irq];
    if (ite.handler)
        ite.handler(irq, ite.arg);
}

void unregister_interrupt(unsigned irq)
{
    // TODO: register kpanic as a default isr, like the old system
    InterruptVector[irq].handler = (isr_t) 0;
    InterruptVector[irq].arg = (void*) 0;
}
