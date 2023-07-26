/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-11
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/platform.h
  \author   Franck Wajsburt
  \brief    Generic platform-specific initalization prototypes

\*------------------------------------------------------------------------------------------------*/

#ifndef _HAL_PLATFORM_H_
#define _HAL_PLATFORM_H_

/**
 * \brief     Request the initialization of all the architecture
 *            This function configures each used devices
 *            and configures which IRQ are used and how they are connected to the CPU
 *            thanks to the ICU Mask
 * \param     fdt is the pointer to the flattened device tree
 * \param     tick is the time in cycles between to IRQ for the timer
 * \return    -1 if initialization fails, else 0
 */
extern int arch_init (void *fdt, int tick);

#endif