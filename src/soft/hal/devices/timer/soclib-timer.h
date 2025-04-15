/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-14
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     hal/devices/timer/soclib-timer.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Soclib timer driver

\*------------------------------------------------------------------------------------------------*/

#ifndef _SOCLIB_TIMER_H_
#define _SOCLIB_TIMER_H_

#include <hal/devices/timer.h>

/**
 * \brief   ISR of the soclib timer device
 *          This function is used by iunction soc_timer_init() in hal/soc/almo1-mips/soc.c
 *          More specifically by register_interrupt() to fill the InterruptVector[] in kernel/kirq.c
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

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
