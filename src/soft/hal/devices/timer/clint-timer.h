/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-03-30
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/devices/timer/clint-timer.h
  \author   Nolan Bled
  \brief    CLINT timer driver
            The CLINT used by qemu is based on the one made by SiFive
            Reference: https://sifive.cdn.prismic.io/sifive%2Fc89f6e5a-cf9e-44c3-a3db-04420702dcc1_sifive+e31+manual+v19.08.pdf

\*------------------------------------------------------------------------------------------------*/

#ifndef _CLINT_TIMER_H_
#define _CLINT_TIMER_H_

#include <hal/devices/timer.h>

#define CLINT_MTIMECMP_OFFSET 0x4000
#define CLINT_MTIME_OFFSET    0xbff8

/**
 * \brief   ISR of the CLINT device
 * \param   irq irq linked to the ISR 
 * \param   timer device linked to the ISR
 * \return  nothing
 */
extern void clint_timer_isr(unsigned irq, struct timer_s *timer);

/**
 * \brief See hal/device/timer.h for the function signature
 * timer_init      : initialize the timer device
 * timer_set_tick  : allow the kernel to set the number of ticks
 * timer_set_event : set the event that will be triggered after a timer
 */
extern struct timer_ops_s ClintTimerOps;

#endif
