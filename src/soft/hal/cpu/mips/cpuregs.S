/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-02
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/cpu/mips/cpu_regs.S
  \author   Franck Wajsburt
  \brief    cpu specific assembly that implement access to cpu-specific registers

\*------------------------------------------------------------------------------------------------*/

//--------------------------------------------------------------------------------------------------
// Special registers
//--------------------------------------------------------------------------------------------------


.globl clock // -------------------------- int clock (void) is an external function
clock:                                  // clock is a terminal function then don't use the stack
    mfc0    $2,     $9                  // 32 bits counter in the coprocessor system (c0_$9)
    li      $3,     0                   // 32 bits MSB = 0
    jr      $31                         // get it in $2 and return

.globl cpuid // -------------------------- int cpuid (void)
cpuid:
    mfc0    $2,     $15,    1           // the cpu number is the coprocessor system (c0_$15,1)
    andi    $2,     $2,     0x3FF       // only 11 bits --> 2048 cpu max :-)
    jr      $31                         // get it in $2 and return
