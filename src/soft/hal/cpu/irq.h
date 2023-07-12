/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-10
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/cpu/irq.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Generic IRQ functions prototypes

\*------------------------------------------------------------------------------------------------*/

#ifndef _HAL_CPU_IRQ_H_
#define _HAL_CPU_IRQ_H_

#define MAX_N_IRQS 1024

/**
 * Generic IRQs functions
 */

/** 
 * \brief   enable irq (do not change the MIPS mode thus stay in kernel mode)
 * \return  nothing
 */
extern void irq_enable (void);

/** 
 * \brief   disable irq 
 * \return  nothing
 */
extern unsigned irq_disable (void);

/**
 * Interrupt table helper functions
 */


/**
 * isr_t is a type of an isr function
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
typedef  void (*isr_t) (unsigned, void*);

struct ite_s {
    isr_t handler;
    void *arg;
};

// This structure needs to be declared in platform-specific initialization file
extern struct ite_s interrupt_table[MAX_N_IRQS];

void register_interrupt(unsigned irq, isr_t handler, void *arg);
void route_interrupt(unsigned irq);
void unregister_interrupt(unsigned irq);

#endif