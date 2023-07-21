/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-10
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     drivers/timer/clint-timer.h
  \author   Nolan Bled
  \brief    CLINT timer driver

\*------------------------------------------------------------------------------------------------*/

#ifndef _CLINT_TIMER_H_
#define _CLINT_TIMER_H_

#include <hal/timer.h>

#define CLINT_MTIMECMP_OFFSET 0x4000
#define CLINT_MTIME_OFFSET    0xbff8

void clint_timer_isr(unsigned irq, struct timer_s *timer);

extern struct timer_ops_s clint_timer_ops;

#endif