/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-02
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/cpu/mips/entry.S
  \author   Franck Wajsburt
  \brief    cpu specific assembly code which implement a part of hcpu.h API

\*------------------------------------------------------------------------------------------------*/

#include <common/syscalls.h>

//--------------------------------------------------------------------------------------------------
// Boot code
//--------------------------------------------------------------------------------------------------


.section .boot,"ax"                     // new section: .boot (see https://bit.ly/3gzKWob)
                                        // flags "ax": a -> allocated means section is in memory
                                        //             x -> section contains instructions

boot:                                   // must be 0xBFC0000 for the MIPS

    mtc0    $0,     $12                 // erase $c0_sr thus ERL because except must be 0x80000180
    la      $29,    __kdata_end         // define stack ptr (first address after kdata region)
    la      $26,    kinit               // get address of kinit() function
    la      $4,     __dtb_address
    jr      $26                         // goto kinit()

.section    .kentry,"ax"                // "ax": allocated executable
.org        0x180                       // ktext is 0x80000000 but kentry is 0x80000180


//--------------------------------------------------------------------------------------------------
// kernel entry for all causes,
//--------------------------------------------------------------------------------------------------


kentry:                                 // kernel entry

    mfc0    $26,    $13                 // read CR (Cause Register)
    andi    $26,    $26,    0x3C        // apply cause mask (keep bits 2 to 5)
    li      $27,    0x20                // 0x20 is the syscall code
    beq     $26,    $27, syscall_handler// if XCODE==8, it is a syscall
    beq     $26,    $0,  irq_handler    // if XCODE==0, it is an irq
    j       kpanic                      // else it is an exception


//--------------------------------------------------------------------------------------------------
// Syscall handler
// - ThreadCurrent is the global variable that points to tne current thread structure
// - FIXME draw the stack pointeur usage
//--------------------------------------------------------------------------------------------------

syscall_handler:

    la      $26,    ThreadCurrent       // get the addr of the current thread pointer address
    lw      $26,    ($26)               // get the current thread pointer
    lw      $27,    ($26)               // get the kernel SP of the current thread
    sw      $29,    -4($27)             // save current user SP at top of kernel stack (below MAGIC)
    addiu   $29,    $27, -9*4           // context 9=user_$29+$31+EPC+SR+syscall_code+4_args

    mfc0    $8,     $14                 // $8 <- EPC (addr of syscall instruction)
    mfc0    $9,     $12                 // $9 <- SR (status register)
    addiu   $8,     $8,     4           // $8 <- EPC+4 (return address)
    sw      $31,    7*4($29)            // save $31 because it will be erased
    sw      $8,     6*4($29)            // save EPC+4 (return address of syscall)
    sw      $9,     5*4($29)            // save SR (status register)
    sw      $2,     4*4($29)            // save syscall code (useful for debug message)
    mtc0    $0,     $12                 // SR <- kernel-mode w/o INT (HWI0=0 UM=0 ERL=0 EXL=0 IE=0)

    la      $8,     SyscallVector       // $9 <- table of syscall functions
    andi    $2,     $2,     SYSCALL_NR-1// apply syscall mask
    sll     $2,     $2,     2           // compute syscall index (mutiply by 4)
    addu    $2,     $8,     $2          // $2 <- & SyscallVector[$2]
    lw      $2,     ($2)                // at the end: $2 <- SyscallVector[$2]
    jalr    $2                          // call service function

    lw      $9,     5*4($29)            // get old SR
    lw      $8,     6*4($29)            // get return address of syscall
    lw      $31,    7*4($29)            // restore $31 (return address of syscall function)
    mtc0    $9,     $12                 // restore SR
    mtc0    $8,     $14                 // restore EPC

    lw      $29,    8*4($29)            // restore user stack pointer
    eret                                // return : jr EPC with EXL <- 0


//--------------------------------------------------------------------------------------------------
// not a syscall, maybe an IRQ
// if it is an IRQ then
// - EPC register contains the return address in the interrupted program and SR.EXL = 1
// - save all temporary registers $1 --> $15, $24, $25, HI, LO and $31
// - call the IRQ_handler which will call an ISR
// - restore all temporary register then return
// During the ISR, SR.EXL = 1, thus all interrupts are masked,
// and since the ISR cannot uses syscall, then it is not necessary to save EPC
//--------------------------------------------------------------------------------------------------


irq_handler:

    mfc0    $26,    $12                 // SR (Status Register)
    ext     $26,    $26,    4,      1   // $26 <- User Mode
    move    $27,    $29                 // $27 is the current stack pointer
    beq     $26,    $0,     irq_kernel  // already in kernel mode

irq_user:

    la      $26,    ThreadCurrent       // get the addr of the current thread pointer address
    lw      $26,    ($26)               // get the current thread pointer
    lw      $29,    ($26)               // get the kernel SP of the current thread

irq_kernel:

    addiu   $29,    $29,    -21*4       // 21 registers to save (17 tmp regs+HI+LO+$31 + old $29)
    sw      $27,    20*4($29)           // save the previous stack pointer
    sw      $31,    19*4($29)           // $31 because, it is lost by jal irq_handler
.set noat                               // in order to be allowed to use $1
    sw      $1,     1*4($29)            // noat means no assembly register, then it can be used
.set at                                 // $1 is usable again by the assembler
    sw      $2,     2*4($29)            // 17 other temporary registers
    sw      $3,     3*4($29)
    sw      $4,     4*4($29)
    sw      $5,     5*4($29)
    sw      $6,     6*4($29)
    sw      $7,     7*4($29)
    sw      $8,     8*4($29)
    sw      $9,     9*4($29)
    sw      $10,    10*4($29)
    sw      $11,    11*4($29)
    sw      $12,    12*4($29)
    sw      $13,    13*4($29)
    sw      $14,    14*4($29)
    sw      $15,    15*4($29)
    sw      $24,    16*4($29)
    sw      $25,    17*4($29)
    mflo    $2
    mfhi    $3
    sw      $2,     18*4($29)           // save LO
    sw      $3,     0($29)              // save HI

    jal     isrcall                     // call the right isr

    lw      $3,     0($29)              // HI
    lw      $2,     18*4($29)           // LO
    mthi    $3                          // restore HI
    mtlo    $2                          // restore LO
    lw      $25,    17*4($29)           // restore all temporary registers and $31
    lw      $24,    16*4($29)
    lw      $15,    15*4($29)
    lw      $14,    14*4($29)
    lw      $13,    13*4($29)
    lw      $12,    12*4($29)
    lw      $11,    11*4($29)
    lw      $10,    10*4($29)
    lw      $9,     9*4($29)
    lw      $8,     8*4($29)
    lw      $7,     7*4($29)
    lw      $6,     6*4($29)
    lw      $5,     5*4($29)
    lw      $4,     4*4($29)
    lw      $3,     3*4($29)
    lw      $2,     2*4($29)
.set noat
    lw      $1,     1*4($29)
.set at
    lw      $31,    19*4($29)           // restore $31

    lw      $29,    20*4($29)           // restore the previous stack pointer
    eret                                // j EPC  AND   SR.EXL <= 0
