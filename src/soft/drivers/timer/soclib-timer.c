/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-10
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     drivers/timer/soclib-timer.c
  \author   Franck Wajsburt, Nolan Bled
  \brief    Soclib timer driver

\*------------------------------------------------------------------------------------------------*/

#include <drivers/timer/soclib-timer.h>
#include <kernel/klibc.h>

static void soclib_timer_set_tick(struct timer_s *timer, unsigned tick)
{
    struct soclib_timer_regs_s *regs = 
        (struct soclib_timer_regs_s *) timer->address;
    regs->period = tick;
}

static void soclib_timer_init(struct timer_s *timer, unsigned address, unsigned tick)
{
    timer->address  = address;
    timer->ops      = &soclib_timer_ops;

    struct soclib_timer_regs_s *regs = 
        (struct soclib_timer_regs_s *) timer->address;
    regs->resetirq = 1;                                 // to be sure there won't be a IRQ when timer start
    soclib_timer_set_tick(timer, tick);                 // next period

    regs->mode = (tick) ? 3 : 0;                        // timer ON with IRQ only if (tick != 0)
}

static void soclib_timer_set_event(struct timer_s *timer, void (*f)(void *arg), void *arg)
{
    timer->event.f = f;
    timer->event.arg = arg;
}

struct timer_ops_s soclib_timer_ops = {
    .timer_init = soclib_timer_init,
    .timer_set_event = soclib_timer_set_event,
    .timer_set_tick = soclib_timer_set_tick
};

void soclib_timer_isr (unsigned irq, struct timer_s *timer)
{
    struct soclib_timer_regs_s *regs = 
        (struct soclib_timer_regs_s *) timer->address;
    regs->resetirq = 1;    // IRQ acknoledgement to lower the interrupt signal
    
    if (timer->event.f)
        timer->event.f(timer->event.arg);
}
