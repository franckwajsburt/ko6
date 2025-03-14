/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-25
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/cpu/riscv/cpuregs.S
  \author   Nolan Bled
  \brief    cpu specific assembly code which implement access to cpu-specific registers

\*------------------------------------------------------------------------------------------------*/

#include <hal/cpu/riscv/context.h>

//--------------------------------------------------------------------------------------------------
// Special registers
//--------------------------------------------------------------------------------------------------

.section .text

.globl clock // -------------------------- int clock (void) is an external function
clock:                                  // clock is a terminal function then don't use the stack
    csrr    a0, mcycle                  // lowest 32 bits cycle counter
    csrr    a1, mcycleh                 // highest 32 bits cycle counter
    ret

.globl cpuid // -------------------------- int cpuid (void)
cpuid:
    csrr    a0, mhartid                 // the cpu number is the coprocessor system
    ret
