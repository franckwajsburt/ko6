/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-03-30
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/devices/timer/soclib-timer.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Soclib timer driver

\*------------------------------------------------------------------------------------------------*/

#ifndef _SOCLIB_TIMER_H_
#define _SOCLIB_TIMER_H_

#include <hal/devices/timer.h>

struct soclib_timer_regs_s {
    int value;          ///< timer's counter : +1 each cycle, can be written
    int mode;           ///< timer's mode : bit 0 = ON/OFF ; bit 1 = IRQ enable
    int period;         ///< timer's period between two IRQ
    int resetirq;       ///< address to acknowledge the timer's IRQ
};

/**
 * \brief   ISR of the soclib timer device
 * \param   irq irq linked to the ISR 
 * \param   timer device linked to the ISR
 * \return  nothing
 */
extern void soclib_timer_isr(unsigned irq, struct timer_s *timer);

/**
 * \brief See hal/device/timer.h for the function signature
 * .timer_init      : initialize the timer device
 * .timer_set_tick  : allow the kernel to set the number of ticks
 * .timer_set_event : set the event that will be triggered after a timer
 */
extern struct timer_ops_s SoclibTimerOps;

#endif
