/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     kernel/kirq.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    IRQs functions

\*------------------------------------------------------------------------------------------------*/

#ifndef _KERNEL_IRQ_H_
#define _KERNEL_IRQ_H_

#define MAX_N_IRQS 1024

/**
 * Interrupt table helper functions
 */

/**
 * \brief isr_t is a type of an isr function
 * \param irq   is the irq number for the ICU, not really useful, but sometimes for debug
 * \param dev   pointer to the device structure
 *
 * This comment explains what is a type of function, and how to create one.
 * If we have a function : RETURN_TYPE FUNCTION_NAME ( list of ARG_TYPE ARG_NAME )
 *
 * For example : int erase (double * tab, unsigned size);
 *  * RETURN_TYPE = int          * FUNCTION_NAME = erase
 *  * ARG_TYPE 1  = double       * ARG_TYPE 2    = unsigned
 *  * ARG_NAME 1  = tab          * ARG_NAME 2    = size
 *
 * To create a variable named VF which is a pointer to a function, we must write :
 *    RETURN_TYPE (*VF) ( list of ARG_TYPE ARG_NAME );
 *    be warned that the parentheses around *VF are mandatory,
 *    otherwise gcc recognize a simple declaration of function
 *
 * For example, we can write :
 *    int (*VF) (double * tab, unsigned size); // declaration of VF
 *    VF = erase;                              // initialisation of VF
 *    int x = VF (tab, size);                  // this line is equivalent to
 *    int x = erase (tab, size);               // this one
 *
 * To create a type named TF for the pointer to a function, we must write :
 *    typedef RETURN_TYPE (*TF) ( list of ARG_TYPE ARG_NAME );
 *    the parentheses around *TF are also mandatory,
 *
 * For example, we can write :
 *    typedef int (*TF) (double * tab, unsigned size);
 *    TF VF ;
 *    VF = erase;
 *    int x = VF (tab, size);
 *    int x = erase (tab, size);
 */
typedef  void (*isr_t) (unsigned irq, void *dev);

/** \brief Interrupt Table Entry, describe what to do for a specific IRQ */
struct ite_s {
    isr_t handler;  //< function called for this entry
    void *arg;      //< parameter passed to the function
};

// extern struct ite_s InterruptVector[MAX_N_IRQS];

/**
 * \brief Assign an ISR (Interrupt-Service Routine) to an IRQ
 * \param irq     the irq
 * \param handler the ISR to call
 * \param arg     argument given to the ISR
 */
extern void register_interrupt(unsigned irq, isr_t handler, void *arg);

/**
 * \brief Call the ISR of a given IRQ
 * \param irq the irq
 */
extern void route_interrupt(unsigned irq);

/**
 * \brief Remove the ISR linked to an IRQ
 * \param irq the irq
 */
extern void unregister_interrupt(unsigned irq);

#endif

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
