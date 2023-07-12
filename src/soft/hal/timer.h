/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-10
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/timer.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Timer API

\*------------------------------------------------------------------------------------------------*/

#ifndef _HAL_TIMER_H_
#define _HAL_TIMER_H_

struct timer_s;

struct timer_ops_s {
    void (*timer_init)(struct timer_s *timer, unsigned address, unsigned tick);
    void (*timer_set_tick)(struct timer_s *timer, unsigned tick);
    void (*timer_set_event)(struct timer_s *timer, void (*f)(void *arg), void *arg);
};

struct timer_event_s {
    void (*f)(void *arg);
    void *arg;
};

struct timer_s {
    unsigned address;
    struct timer_event_s event;
    struct timer_ops_s *ops;
};

extern struct timer_s timer;

#endif