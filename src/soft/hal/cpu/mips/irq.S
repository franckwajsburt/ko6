/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-25
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/cpu/mips/irq.S
  \author   Franck Wajsburt
  \brief    cpu specific assembly code which implement IRQs functions

\*------------------------------------------------------------------------------------------------*/

//--------------------------------------------------------------------------------------------------
// IRQ mask operations
//--------------------------------------------------------------------------------------------------

.section .text

.globl irq_enable// --------------------- void irq_enable(void)
irq_enable:
    li      $2,     0x401               // SR <-- 0x401 : IM=0x04 UM=0 EXL=0 IE=1
    mtc0    $2,     $12
    jr      $31

.globl irq_disable// -------------------- unsigned irq_disable(void)
irq_disable:
    mfc0    $2,     $12                 // get SR
    mtc0    $0,     $12                 // SR <-- 0 : IM=0x00 UM=0 EXL=0 IE=0
    jr      $31
