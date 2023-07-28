/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-21
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     drivers/timer/clint-timer.c
  \author   Nolan Bled
  \brief    CLINT timer driver

\*------------------------------------------------------------------------------------------------*/

#include <drivers/timer/clint-timer.h>

static void clint_timer_set_tick(struct timer_s *timer, unsigned tick)
{
    // We have to options: either set mtimecmp, or reset mtime
    *(unsigned*) (timer->address + CLINT_MTIMECMP_OFFSET) =
        (*(unsigned*) (timer->address + CLINT_MTIME_OFFSET)) + tick;
    timer->period = tick;
}

static void clint_timer_init(struct timer_s *timer, unsigned address, unsigned tick)
{
    timer->address  = address;
    timer->ops      = &clint_timer_ops;

    clint_timer_set_tick(timer, tick); // next period
}

static void clint_timer_set_event(struct timer_s *timer, void (*f)(void *arg), void *arg)
{
    timer->event.f = f;
    timer->event.arg = arg;
}

struct timer_ops_s clint_timer_ops = {
    .timer_init = clint_timer_init,
    .timer_set_event = clint_timer_set_event,
    .timer_set_tick = clint_timer_set_tick
};

void clint_timer_isr (unsigned irq, struct timer_s *timer)
{
    // Reset timer
    timer->ops->timer_set_tick(timer, timer->period);

    if (timer->event.f)
        timer->event.f(timer->event.arg);
}
