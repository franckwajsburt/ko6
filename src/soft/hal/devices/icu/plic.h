/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-08-01
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/devices/icu/plic.h
  \author   Nolan Bled
  \brief    PLIC driver 

\*------------------------------------------------------------------------------------------------*/

#ifndef _PLIC_H_
#define _PLIC_H_

#include <kernel/klibc.h>
#include <hal/devices/icu.h>

/**
 * \brief Return the M-context no based on the cpu no
 *        Context are a pair (cpu, mode)
 *        On qemu, M-mode are even number and S-mode odd, by exemple:
 *          (cpu0, M-mode) is context 0
 *          (cpu0, S-mode) is context 1
 *          (cpu1, M-mode) is context 2, ect ...
 */
#define PLIC_M_CONTEXT(cpu) (2*(cpu))
/** \brief Return the S-context no based on the cpu no */
#define PLIC_S_CONTEXT(cpu) (2*(cpu)+1)

/**
 * Memory map:
 * See https://github.com/riscv/riscv-plic-spec/blob/master/riscv-plic.adoc for details
 * base + 0x000000 to base + 0x001000: interrupts 1-1023 priority
 * base + 0x001000 to base + 0x00107C: pending bits 1-1023
 * 
 * base + 0x002000 to base + 0x00207C: enable bits 1-1023 for context 0
 * base + 0x002080 to base + 0x0020FC: enable bits 1-1023 for context 1
 *
 * base + 0x200000: priority treshold for context 0
 * base + 0x200004: int. claim/complete for context 0
 * base + 0x201000: priority treshold for context 1
 * base + 0x201004: int. claim/complete for context 1
 */
#define PLIC_PRI_OFFSET         0x000000
#define PLIC_PENDING_OFFSET     0x001000
#define PLIC_ENABLE_OFFSET      0x002000
#define PLIC_PRI_TRESH_OFFSET   0x200000
#define PLIC_CLAIM_OFFSET       0x200004

// Inspired from the xv6 macros
#define PLIC_MENABLE(base, cpu, irq) ((base) + \
                                     PLIC_ENABLE_OFFSET + \
                                     PLIC_M_CONTEXT(cpu) * 0x80 + \
                                     (irq) / 32)
#define PLIC_SENABLE(base, cpu, irq) ((base) + \
                                     PLIC_ENABLE_OFFSET + \
                                     PLIC_S_CONTEXT(cpu) * 0x80 + \
                                     (irq) / 32)

#define PLIC_MPRITRESH(base, cpu)    ((base) + \
                                     PLIC_PRI_TRESH_OFFSET + \
                                     PLIC_M_CONTEXT(cpu) * 0x1000)
#define PLIC_SPRITRESH(base, cpu)    ((base) + \
                                     PLIC_PRI_TRESH_OFFSET + \
                                     PLIC_S_CONTEXT(cpu) * 0x1000)

#define PLIC_MCLAIM(base, cpu)       ((base) + \
                                     PLIC_CLAIM_OFFSET + \
                                     PLIC_M_CONTEXT(cpu) * 0x1000)
#define PLIC_SCLAIM(base, cpu)       ((base) + \
                                     PLIC_CLAIM_OFFSET + \
                                     PLIC_S_CONTEXT(cpu) * 0x1000)

/**
 * \brief See hal/device/icu.h for the function signature
 * .icu_init         : initialize the ICU device                           
 * .icu_get_highest  : fetch the highest priority IRQ from the ICU device
 * .icu_acknowledge  : should aknowledge an IRQ
 * .icu_mask         : mask (disable) an IRQ
 * .icu_unmask       : unmask (enable) an IRQ
 * .icu_set_priority : sets the priority of a given IRQ                    
 */
extern struct icu_ops_s PlicOps;

#endif
