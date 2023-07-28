/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-25
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     platforms/virt/cpu/thread.S
  \author   Nolan Bled
  \brief    cpu specific assembly code which implement a part of cpu HAL
\*------------------------------------------------------------------------------------------------*/

#include <platforms/qemu-virt-riscv/cpu.h>

//--------------------------------------------------------------------------------------------------
// Thread management
//--------------------------------------------------------------------------------------------------

.section .text

.globl thread_context_save // ------------ int thread_save (int context[])
thread_context_save:                    // saves all persistant registers and returns 1
    // save callee-saved general registers
    sw      s0,     TH_CONTEXT_S0*4(a0)
    sw      s1,     TH_CONTEXT_S1*4(a0)
    sw      s2,     TH_CONTEXT_S2*4(a0)
    sw      s3,     TH_CONTEXT_S3*4(a0)
    sw      s4,     TH_CONTEXT_S4*4(a0)
    sw      s5,     TH_CONTEXT_S5*4(a0)
    sw      s6,     TH_CONTEXT_S6*4(a0)
    sw      s7,     TH_CONTEXT_S7*4(a0)
    sw      s8,     TH_CONTEXT_S8*4(a0)
    sw      s9,     TH_CONTEXT_S9*4(a0)
    sw      s10,    TH_CONTEXT_S10*4(a0)
    sw      s11,    TH_CONTEXT_S11*4(a0)
    sw      ra,     TH_CONTEXT_RA*4(a0)// save RA that is the return address of thread_save
    sw      sp,     TH_CONTEXT_SP*4(a0)// save current stack pointer

    // save useful csrs
    csrr    t0,     mstatus
    csrr    t1,     mepc// get MEPC that contains the return addr to the old thread

    sw      t0,     TH_CONTEXT_MSTATUS*4(a0)
    sw      t1,     TH_CONTEXT_MEPC*4(a0)

    // return 1 
    li      a0,     1

    // returns to the thread_save caller
    ret

.globl thread_context_load // ------------ int thread_load (int context[])
thread_context_load:                    // load all persistant regs and returns 0 of thread_save
    // load callee-saved general registers
    lw      s0,     TH_CONTEXT_S0*4(a0)
    lw      s1,     TH_CONTEXT_S1*4(a0)
    lw      s2,     TH_CONTEXT_S2*4(a0)
    lw      s3,     TH_CONTEXT_S3*4(a0)
    lw      s4,     TH_CONTEXT_S4*4(a0)
    lw      s5,     TH_CONTEXT_S5*4(a0)
    lw      s6,     TH_CONTEXT_S6*4(a0)
    lw      s7,     TH_CONTEXT_S7*4(a0)
    lw      s8,     TH_CONTEXT_S8*4(a0)
    lw      s9,     TH_CONTEXT_S9*4(a0)
    lw      s10,    TH_CONTEXT_S10*4(a0)
    lw      s11,    TH_CONTEXT_S11*4(a0)
    lw      ra,     TH_CONTEXT_RA*4(a0) // return addr of thread_load which is that of thread_save
    lw      sp,     TH_CONTEXT_SP*4(a0) // define the next stack pointer

    // load csrs
    lw      t0,     TH_CONTEXT_MSTATUS*4(a0)
    lw      t1,     TH_CONTEXT_MEPC*4(a0)

    csrw    mstatus,    t0
    csrw    mepc,       t1

    // return 0
    li      a0,     0

    // return to the thread_save caller (and not thread_load)
    ret

.globl thread_launch // ---------- void thread_launch (void *(*fun)(void *), void *arg, void *start)
thread_launch:                  // calls either _start() or thread_start(), with args in a0 and a1
    csrw    mepc, a2            // jump to start function
    
    li      t0, 3 << 11 
    csrc    mstatus, t0         // clear the mstatus.MPP bits, 
                                // to set mstatus.MPP = U mode
    li      t0, 1 << 7
    csrs    mstatus, t0         // set mstatus.MPIE bit, to enable
                                // interrupts after the switch to M-mode

    mret                        // return to M-mode
