/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     hal/devices/timer/clint-timer.c
  \author   Nolan Bled
  \brief    CLINT timer driver

\*------------------------------------------------------------------------------------------------*/

#include <hal/devices/timer/clint-timer.h>

/**
 * \brief   Set the number of ticks between two CLINT timer interrupts 
 * \param   timer timer device
 * \param   tick number of ticks
 * \return  nothing
 */
static void clint_timer_set_tick (timer_t *timer, unsigned tick)
{
    // We have to options: either set mtimecmp, or reset mtime
    *(unsigned*) (timer->base + CLINT_MTIMECMP_OFFSET) =
        (*(unsigned*) (timer->base + CLINT_MTIME_OFFSET)) + tick;
    timer->period = tick;
}

/**
 * \brief   CLINT timer initialization 
 * \param   timer   timer device to initialize
 * \param   minor   minor number is the device instance number
 * \param   base    The base address of the device
 * \param   tick    number of ticks between two interrupts
 * \return  nothing
 */
static void clint_timer_init (timer_t *timer, unsigned minor, unsigned base, unsigned tick)
{
    timer->base  = base;
    timer->minor = minor;
    timer->ops   = &ClintTimerOps;

    clint_timer_set_tick (timer, tick); // next period
}

/**
 * \brief   Set the event that will triggered by a CLINT interrupt
 * \param   timer the timer device
 * \param   f the function corresponding to the event
 * \param   arg argument that will be passed to the function
 * \return  nothing
 */
static void clint_timer_set_event (timer_t *timer, void (*f)(void *arg), void *arg)
{
    timer->event.f = f;
    timer->event.arg = arg;
}

struct timer_ops_s ClintTimerOps = {
    .timer_init = clint_timer_init,
    .timer_set_event = clint_timer_set_event,
    .timer_set_tick = clint_timer_set_tick
};

void clint_timer_isr (unsigned irq, timer_t *timer)
{
    /* Reset timer */
    timer->ops->timer_set_tick (timer, timer->period);

    /* If a function is available, trigger the event */
    if (timer->event.f)
        timer->event.f (timer->event.arg);
}
