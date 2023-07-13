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

    /**
     * \brief   Generic function that allows the kernel to set the number of ticks
     *          after which we want an interrupt
     * \param   timer the timer device
     * \param   tick  the number of ticks
     * \return  nothing
    */
    void (*timer_set_tick)(struct timer_s *timer, unsigned tick);

    /**
     * \brief   Generic function that sets the event that will be triggered after a timer
     *          interrupt
     * \param   timer   the timer device
     * \param   f       the function to be called
     * \param   arg     the arg passed to the function
     * \return  nothing
    */
    void (*timer_set_event)(struct timer_s *timer, void (*f)(void *arg), void *arg);
};

/**
 * Structure describing what to do when we receive a timer interrupt
 */
struct timer_event_s {
    void (*f)(void *arg);           // function triggered
    void *arg;                      // argument passed to the function
};

struct timer_s {
    unsigned address;               // timer's address
    struct timer_event_s event;     // event triggered each nticks
    struct timer_ops_s *ops;        // driver-specific operations
};
#define timer_alloc() (struct timer_s*) (dev_alloc(TIMER_DEV, sizeof(struct timer_s))->data)
#define timer_get(no) (struct timer_s*) (dev_get(TIMER_DEV, no)->data)

#endif