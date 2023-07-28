/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-26
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     platforms/virt/cpu/entry.S
  \author   Nolan Bled
  \brief    cpu specific assembly code which implement a part of cpu HAL

\*------------------------------------------------------------------------------------------------*/

#include <common/syscalls.h>

//--------------------------------------------------------------------------------------------------
// Boot code
//--------------------------------------------------------------------------------------------------

.section .boot,"ax"                     // new section: .boot (see https://bit.ly/3gzKWob)
                                        // flags "ax": a -> allocated means section is in memory
                                        //             x -> section contains instructions

boot:                                   // qemu put pc at 0x80000000
    la      t0,     1 << 3
    csrc    mstatus,    t0              // disable interrupts (mstatus.MIE = 0)

    // mie.MTIE = 1, mie.MEIE = 1, mie.MSIE = 1
    // it will enable all interrupts when mstatus.MIE = 1
    la      t0,     1 << 3 | 1 << 7 | 1 << 11
    csrs    mie,        t0

    la      t0,     kentry
    csrw    mtvec,  t0                  // set interrupt vector
    
    /* TODO: use pmp to protect kernel area */
    la      t0,     0x201fffff          // 0x80000000 - 0x81000000
    csrw    pmpaddr0, t0
    la      t1, 0b00011000         // NAPOT ---

    // we need to disable protection on the memory
    // not occupied by the kernel because by default
    // U-mode cannot address any memory
    not     t0, x0
    csrw    pmpaddr1, t0
    la      t0, 0b00011111         // NAPOT RWX
    slli    t0, t0, 8
    or      t0, t1, t0
    csrw    pmpcfg0, t0

    la      sp,     __kdata_end         // define stack ptr (first address after kdata region)
    mv      a0,     a1                  // qemu put fdt address in a1, we give it to kinit as a first argument
    call    kinit

.section .text

//--------------------------------------------------------------------------------------------------
// kernel entry for all causes,
//--------------------------------------------------------------------------------------------------

kentry:                                 // kernel entry
    // save temp register: since there's no kernel register in riscv
    // we need to save temp register in a trap so we can do operations
    // they'll be restored afterwards
    addi    sp, sp, -8
    sw      t0, 0(sp)
    sw      t1, 4(sp)

    // check if mcause == 8 (env. call from U-mode)
    csrr    t0, mcause
    la      t1, 8
    beq     t0, t1, syscall_handler

    // check if first bit == 1 (interrupt)
    srli    t0, t0, 31
    bnez    t0, irq_handler

    j kpanic

//--------------------------------------------------------------------------------------------------
// Syscall handler
// - ThreadCurrent is the global variable that points to tne current thread structure
// - FIXME draw the stack pointeur usage
//--------------------------------------------------------------------------------------------------

syscall_handler:
    // there's no need to restore t0/t1 since this is a syscall
    // we still need to restore sp
    addi    sp, sp, 8
    
    la      t0, ThreadCurrent   // get the addr of the current thread pointer address
    lw      t0, 0(t0)           // get the current thread pointer
    lw      t0, 0(t0)           // get the kernel SP of the current thread
    sw      sp, -4(t0)          // save current user SP at top of kernel stack (below MAGIC)
    addi    sp, t0, -3*4        // switch to kernel stack, and allocate space for:
                                // user_sp+ra+mepc

    csrr    t0, mepc            // t0 <- EPC (addr of syscall instruction)
    addi    t0, t0, 4           // t0 <- MEPC+4 (return address)
    sw      ra, 1*4(sp)         // save ra because it will be erased
    sw      t0, 0*4(sp)         // save MEPC+4 (return address of syscall)

    la      t0, SyscallVector       // t0 <- table of syscall functions
    andi    t1, a4, SYSCALL_NR-1    // apply syscall mask
    sll     t1, t1, 2               // compute syscall index (mutiply by 4)
    add     t1, t0, t1              // a0 <- & SyscallVector[a0]
    lw      t1, (t1)                // at the end: a0 <- SyscallVector[a0]
    jalr    t1                      // call service function

    lw      t0, 0*4(sp)             // get return address of syscall
    lw      ra, 1*4(sp)             // restore $31 (return address of syscall function)
    csrw    mepc, t0                // restore MEPC

    lw      sp, 2*4(sp)             // restore user stack pointer
    mret

//--------------------------------------------------------------------------------------------------
// not a syscall, maybe an IRQ
// if it is an IRQ then
// - EPC register contains the return address in the interrupted program and SR.EXL = 1
// - save all temporary registers t0-t6, ra, a0-a7
// - call the IRQ_handler which will call an ISR
// - restore all temporary register then return
// Since the ISR cannot uses syscall, then it is not necessary to save EPC
//--------------------------------------------------------------------------------------------------

irq_handler:
    csrr    t0, mstatus
    srli    t0, t0, 11
    andi    t0, t0, 3       // extract mstatus.MPP to know previous privilege mode

    mv      t1, sp          // t1 = current sp
    bnez    t0, irq_kernel  // mstatus.MPP != 0, we were already in kernel mode

irq_user:
    la      t0, ThreadCurrent
    lw      t0, 0(t0)
    lw      sp, 0(t0)

irq_kernel:
    addi    t1, t1, 8    // restore correct previous sp
    addi    sp, sp, -17*4// 17 registers to save (a0-a7, t0-t6, previous sp, ra)

    sw      t1, 16*4(sp)// save the correct previous sp
    addi    t1, t1, -8  // restore t0/t1
    lw      t0, 0(t1)
    lw      t1, 4(t1)

    sw      ra, 15*4(sp)// ra because, it is lost by jal irq_handler
    sw      a0, 14*4(sp)
    sw      a1, 13*4(sp)
    sw      a2, 12*4(sp)
    sw      a3, 11*4(sp)
    sw      a4, 10*4(sp)
    sw      a5, 9*4(sp)
    sw      a6, 8*4(sp)
    sw      a7, 7*4(sp)
    sw      t0, 6*4(sp)
    sw      t1, 5*4(sp)
    sw      t2, 4*4(sp)
    sw      t3, 3*4(sp)
    sw      t4, 2*4(sp)
    sw      t5, 1*4(sp)
    sw      t6, 0*4(sp)

    csrr    a0, mcause
    jal     isrcall     // call the right isr

    // restore all temporary registers and ra
    lw      ra, 15*4(sp)// ra because, it is lost by jal irq_handler
    lw      a0, 14*4(sp)
    lw      a1, 13*4(sp)
    lw      a2, 12*4(sp)
    lw      a3, 11*4(sp)
    lw      a4, 10*4(sp)
    lw      a5, 9*4(sp)
    lw      a6, 8*4(sp)
    lw      a7, 7*4(sp)
    lw      t0, 6*4(sp)
    lw      t1, 5*4(sp)
    lw      t2, 4*4(sp)
    lw      t3, 3*4(sp)
    lw      t4, 2*4(sp)
    lw      t5, 1*4(sp)
    lw      t6, 0*4(sp)

    lw      sp, 16*4(sp)//restore the previous stack pointer
    mret