/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-10
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/devices/timer/soclib-timer.c
  \author   Franck Wajsburt, Nolan Bled
  \brief    Soclib timer driver

\*------------------------------------------------------------------------------------------------*/

#include <hal/devices/timer/soclib-timer.h>
#include <kernel/klibc.h>

/**
 * \brief   Set the number of ticks between two soclib timer interrupts 
 * \param   timer timer device
 * \param   tick number of ticks
 * \return  nothing
 */
static void soclib_timer_set_tick(struct timer_s *timer, unsigned tick)
{
    struct soclib_timer_regs_s *regs = (struct soclib_timer_regs_s *) timer->base;
    regs->period = tick;
}

/**
 * \brief   soclib timer initialization 
 * \param   timer   timer device to initialize
 * \param   base    The base address of the device
 * \param   tick    number of ticks between two interrupts
 * \return  nothing
 */
static void soclib_timer_init(struct timer_s *timer, unsigned base, unsigned tick)
{
    timer->base  = base;
    timer->ops      = &SoclibTimerOps;

    struct soclib_timer_regs_s *regs = (struct soclib_timer_regs_s *) timer->base;
    regs->resetirq = 1;                       // to be sure there won't be a IRQ when timer start
    soclib_timer_set_tick(timer, tick);       // next period

    regs->mode = (tick) ? 3 : 0;              // timer ON with IRQ only if (tick != 0)
}

/**
 * \brief   Set the event that will triggered by a soclib timer interrupt
 * \param   timer the timer device
 * \param   f the function corresponding to the event
 * \param   arg argument that will be passed to the function
 * \return  nothing
 */
static void soclib_timer_set_event(struct timer_s *timer, void (*f)(void *arg), void *arg)
{
    timer->event.f = f;
    timer->event.arg = arg;
}

struct timer_ops_s SoclibTimerOps = {
    .timer_init = soclib_timer_init,
    .timer_set_event = soclib_timer_set_event,
    .timer_set_tick = soclib_timer_set_tick
};

void soclib_timer_isr (unsigned irq, struct timer_s *timer)
{
    struct soclib_timer_regs_s *regs = 
        (struct soclib_timer_regs_s *) timer->base;
    regs->resetirq = 1;                     // IRQ acknoledgement to lower the interrupt signal
    
    if (timer->event.f)
        timer->event.f(timer->event.arg);
}
