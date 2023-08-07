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

/**
 * \brief   Set the number of ticks between two CLINT timer interrupts 
 * \param   timer timer device
 * \param   tick number of ticks
 * \return  nothing
 */
static void clint_timer_set_tick(struct timer_s *timer, unsigned tick)
{
    // We have to options: either set mtimecmp, or reset mtime
    *(unsigned*) (timer->address + CLINT_MTIMECMP_OFFSET) =
        (*(unsigned*) (timer->address + CLINT_MTIME_OFFSET)) + tick;
    timer->period = tick;
}

/**
 * \brief   CLINT timer initialization 
 * \param   timer timer device to initialize
 * \param   address address of the device
 * \param   tick number of ticks between two interrupts
 * \return  nothing
 */
static void clint_timer_init(struct timer_s *timer, unsigned address, unsigned tick)
{
    timer->address  = address;
    timer->ops      = &ClintTimerOps;

    clint_timer_set_tick(timer, tick); // next period
}

/**
 * \brief   Set the event that will triggered by a CLINT interrupt
 * \param   timer the timer device
 * \param   f the function corresponding to the event
 * \param   arg argument that will be passed to the function
 * \return  nothing
 */
static void clint_timer_set_event(struct timer_s *timer, void (*f)(void *arg), void *arg)
{
    timer->event.f = f;
    timer->event.arg = arg;
}

struct timer_ops_s ClintTimerOps = {
    .timer_init = clint_timer_init,
    .timer_set_event = clint_timer_set_event,
    .timer_set_tick = clint_timer_set_tick
};

void clint_timer_isr (unsigned irq, struct timer_s *timer)
{
    /* Reset timer */
    timer->ops->timer_set_tick(timer, timer->period);

    /* If a function is available, trigger the event */
    if (timer->event.f)
        timer->event.f(timer->event.arg);
}
