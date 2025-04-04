/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-03-30
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/devices/timer.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Timer API

\*------------------------------------------------------------------------------------------------*/

#ifndef _HAL_TIMER_H_
#define _HAL_TIMER_H_

struct timeri_ops_s;

/** \brief Structure describing what to do when we receive a timer interrupt */
struct timer_event_s {
    void (*f)(void *arg);           ///< function triggered
    void *arg;                      ///< argument passed to the function
};

/** \brief Timer driver informations */
struct timer_s {
    unsigned base;                  ///< timer's base address
    unsigned period;                ///< number of ticks between two events
    struct timer_event_s event;     ///< event triggered each nticks
    struct timer_ops_s *ops;        ///< driver-specific operations
};

/** 
 * \brief Functions prototypes of the timer device, they should be implemented by a device driver. 
 *        They serve as an interface between the kernel and the driver
 */
struct timer_ops_s {
    /**
     * \brief   Generic function to initialize the timer device
     * \param   base the base address of the memory-mapped registers
     * \param   tick number of ticks between to timer interrupts
     */
    void (*timer_init)(struct timer_s *timer, unsigned base, unsigned tick);

    /**
     * \brief   Generic function that allows the kernel to set the number of ticks
     *          after which we want an interrupt
     * \param   timer the timer device
     * \param   tick  the number of ticks
    */
    void (*timer_set_tick)(struct timer_s *timer, unsigned tick);

    /**
     * \brief   Generic function that sets the event that will be triggered after a timer
     *          interrupt
     * \param   timer   the timer device
     * \param   f       the function to be called
     * \param   arg     the arg passed to the function
    */
    void (*timer_set_event)(struct timer_s *timer, void (*f)(void *arg), void *arg);
};

#define timer_alloc() (struct timer_s*) (dev_alloc(TIMER_DEV, sizeof(struct timer_s))->data)
#define timer_get(no) (struct timer_s*) (dev_get(TIMER_DEV, no)->data)
#define timer_count() (dev_next_no(TIMER_DEV) - 1)

#endif
