/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-02
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/cpu/riscv/kpanica.S
  \author   Nolan Bled
  \brief    cpu specific assembly code which implement kpanic()

\*------------------------------------------------------------------------------------------------*/

#include <hal/cpu/riscv/context.h>

//--------------------------------------------------------------------------------------------------
// Not managed cause, thus call kpanic () that displays the registers' value and stops the program
//--------------------------------------------------------------------------------------------------

.globl kpanic // ------------------------- void kpanic (void)
kpanic:
    // Save registers in stack so we can use them
    addi    sp, sp, -4
    sw      t0, 0(sp)

    // save all registers in KPanicRegsVal[]
    la      t0, KPanicRegsVal

    sw      ra, KPANIC_RA*4(t0)
    sw      gp, KPANIC_GP*4(t0)
    sw      tp, KPANIC_TP*4(t0)
    // don't save t0 yet since we overwrote it
    sw      t1, KPANIC_T1*4(t0)
    sw      t2, KPANIC_T2*4(t0)
    sw      s0, KPANIC_S0*4(t0)
    sw      s1, KPANIC_S1*4(t0)
    sw      a0, KPANIC_A0*4(t0)
    sw      a1, KPANIC_A1*4(t0)
    sw      a2, KPANIC_A2*4(t0)
    sw      a3, KPANIC_A3*4(t0)
    sw      a4, KPANIC_A4*4(t0)
    sw      a5, KPANIC_A5*4(t0)
    sw      a6, KPANIC_A6*4(t0)
    sw      a7, KPANIC_A7*4(t0)
    sw      s2, KPANIC_S2*4(t0)
    sw      s3, KPANIC_S3*4(t0)
    sw      s4, KPANIC_S4*4(t0)
    sw      s5, KPANIC_S5*4(t0)
    sw      s6, KPANIC_S6*4(t0)
    sw      s7, KPANIC_S7*4(t0)
    sw      s8, KPANIC_S8*4(t0)
    sw      s9, KPANIC_S9*4(t0)
    sw      s10, KPANIC_S10*4(t0)
    sw      s11, KPANIC_S11*4(t0)
    sw      t3, KPANIC_T3*4(t0)
    sw      t4, KPANIC_T4*4(t0)
    sw      t5, KPANIC_T5*4(t0)
    sw      t6, KPANIC_T6*4(t0)

    // restore t0/sp and save them too
    la      t1, KPanicRegsVal
    lw      t0, 0(sp)
    addi    sp, sp, 4
    sw      t0, KPANIC_T0*4(t1)
    sw      sp, KPANIC_SP*4(t1)

    // save csrs
    csrr    t0, mcycle
    csrr    t2, mtval
    csrr    t3, mstatus
    csrr    t4, mepc

    sw      t0, KPANIC_MCYCLE*4(t1)
    sw      t2, KPANIC_MTVAL*4(t1)
    sw      t3, KPANIC_MSTATUS*4(t1)
    sw      t4, KPANIC_MEPC*4(t1)

    // jump to kdump(KPanicRegsVal)
    csrr    a0, mcause
    mv      a1, t1
    la      t0, kdump
    jr      t0
