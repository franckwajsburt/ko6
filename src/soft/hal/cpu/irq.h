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

/**
 * Generic IRQs functions
 */

/**
 * \brief   enable irq (do not change the MIPS mode thus stay in kernel mode)
 */
extern void irq_enable (void);

/**
 * \brief   disable irq
 */
extern unsigned irq_disable (void);

#endif
