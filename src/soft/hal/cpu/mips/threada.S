/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-25
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/cpu/mips/threada.S
  \author   Franck Wasjburt
  \brief    cpu specific assembly code which implement threading functions
\*------------------------------------------------------------------------------------------------*/

#include <hal/cpu/mips/context.h>

//--------------------------------------------------------------------------------------------------
// Thread management
//--------------------------------------------------------------------------------------------------

.section .text

.globl thread_context_save // ------------ int thread_save (int context[])
thread_context_save:                    // saves all persistant registers and returns 1
    mfc0    $26,    $12                 // get STATUS register
    mfc0    $27,    $14                 // get EPC that contains the return addr to the old thread
    sw      $16,    TH_CONTEXT_S0*4($4) // $16 to $23 and $30 are persistant General Purpose Regs
    sw      $17,    TH_CONTEXT_S1*4($4)
    sw      $18,    TH_CONTEXT_S2*4($4)
    sw      $19,    TH_CONTEXT_S3*4($4)
    sw      $20,    TH_CONTEXT_S4*4($4)
    sw      $21,    TH_CONTEXT_S5*4($4)
    sw      $22,    TH_CONTEXT_S6*4($4)
    sw      $23,    TH_CONTEXT_S7*4($4)
    sw      $30,    TH_CONTEXT_S8*4($4)
    sw      $26,    TH_CONTEXT_SR*4($4) // save current STATUS register (U/K and IRQ mode)
    sw      $27,    TH_CONTEXT_EPC*4($4)// save EPC
    sw      $31,    TH_CONTEXT_RA*4($4) // save $31 that is the return address of thread_save
    sw      $29,    TH_CONTEXT_SP*4($4) // save current stack pointer
    li      $2,     1                   // return 1
    jr      $31                         // returns to the thread_save caller

.globl thread_context_load // ------------ int thread_load (int context[])
thread_context_load:                    // load all persistant regs and returns 0 of thread_save
    lw      $2,     TH_TID*4($4)        // used by tracelog, to know which is the new thread
    lw      $16,    TH_CONTEXT_S0*4($4) // restore all peristant registers: $16 to $23 and $30
    lw      $17,    TH_CONTEXT_S1*4($4)
    lw      $18,    TH_CONTEXT_S2*4($4)
    lw      $19,    TH_CONTEXT_S3*4($4)
    lw      $20,    TH_CONTEXT_S4*4($4)
    lw      $21,    TH_CONTEXT_S5*4($4)
    lw      $22,    TH_CONTEXT_S6*4($4)
    lw      $23,    TH_CONTEXT_S7*4($4)
    lw      $30,    TH_CONTEXT_S8*4($4)
    lw      $26,    TH_CONTEXT_SR*4($4) // get the next STATUS register value
    lw      $27,    TH_CONTEXT_EPC*4($4)// get the next EPC : the return addr in the new thread
    lw      $31,    TH_CONTEXT_RA*4($4) // return addr of thread_load which is that of thread_save
    lw      $29,    TH_CONTEXT_SP*4($4) // define the next stack pointer
    mtc0    $26,    $12                 // set STATUS register (U/K and IRQ mode)
    mtc0    $27,    $14                 // set EPC
    li      $2,     0                   // return 0
    jr      $31                         // return to the thread_save caller (and not thread_load)

.globl thread_launch // ---------- void thread_launch (void *(*fun)(void *), void *arg, void *start)
thread_launch:                  // calls either _start() or thread_start(), with args in $4 and $5
    mtc0    $6,     $14         // $6 contains the pointer of the right start function --> EPC
    eret                        // PC <- EPC  and SR.EXL <- 0
