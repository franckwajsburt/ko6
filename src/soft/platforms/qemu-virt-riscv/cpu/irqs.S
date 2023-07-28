/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-25
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     platforms/virt/cpu/irq.S
  \author   Nolan Bled
  \brief    cpu specific assembly code which implement a part of cpu HAL

\*------------------------------------------------------------------------------------------------*/

//--------------------------------------------------------------------------------------------------
// IRQ mask operations
//--------------------------------------------------------------------------------------------------

.section .text

.globl irq_enable// --------------------- void irq_enable(void)
irq_enable:
    csrs mstatus, 1 << 3    // set the mstatus.MIE bit
                            // this will enable the interrupts specified in the
                            // mie csr, which is setup at boot time to enable
                            // timer interrupts (MTIE), software interrupts (MSIE)
                            // external interrupts (MEIE)
                            // note: software interrupts actually relate to IPC
                            //  and have nothing to do with syscalls/exceptions
    ret

.globl irq_disable// -------------------- unsigned irq_disable(void)
irq_disable:
    csrr a0, mstatus        // return the original mstatus (won't be useful
                            // on the riscv implem so i won't implement the 
                            // irq_restore function)
    csrc mstatus, 1 << 3    // clear the mstatus.MIE bit
    ret
