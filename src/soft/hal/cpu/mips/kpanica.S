/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-02
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/cpu/mips/kpanica.S
  \author   Franck Wajsburt
  \brief    cpu specific assembly code which implement kpanic()

\*------------------------------------------------------------------------------------------------*/

#include <hal/cpu/mips/context.h>

//--------------------------------------------------------------------------------------------------
// Not managed cause, thus call kpanic () that displays the registers' value and stops the program
//--------------------------------------------------------------------------------------------------

.globl kpanic // ------------------------- void kpanic (void)
kpanic:

    addiu   $29,    $29,   -4           // reserve space for arg of kdump()

    la      $26,    KPanicRegsVal       // save all registers in KPanicRegsVal[]
.set noat
    sw      $1,     KPANIC_AT*4 ($26)
.set at
    sw      $2,     KPANIC_V0*4 ($26)
    sw      $3,     KPANIC_V1*4 ($26)
    sw      $4,     KPANIC_A0*4 ($26)
    sw      $5,     KPANIC_A1*4 ($26)
    sw      $6,     KPANIC_A2*4 ($26)
    sw      $7,     KPANIC_A3*4 ($26)
    sw      $8,     KPANIC_T0*4 ($26)
    sw      $9,     KPANIC_T1*4 ($26)
    sw      $10,    KPANIC_T2*4 ($26)
    sw      $11,    KPANIC_T3*4 ($26)
    sw      $12,    KPANIC_T4*4 ($26)
    sw      $13,    KPANIC_T5*4 ($26)
    sw      $14,    KPANIC_T6*4 ($26)
    sw      $15,    KPANIC_T7*4 ($26)
    sw      $16,    KPANIC_S0*4 ($26)
    sw      $17,    KPANIC_S1*4 ($26)
    sw      $18,    KPANIC_S2*4 ($26)
    sw      $19,    KPANIC_S3*4 ($26)
    sw      $20,    KPANIC_S4*4 ($26)
    sw      $21,    KPANIC_S5*4 ($26)
    sw      $22,    KPANIC_S6*4 ($26)
    sw      $23,    KPANIC_S7*4 ($26)
    sw      $24,    KPANIC_T8*4 ($26)
    sw      $25,    KPANIC_T9*4 ($26)
    sw      $28,    KPANIC_GP*4 ($26)
    sw      $29,    KPANIC_SP*4 ($26)
    sw      $30,    KPANIC_FP*4 ($26)
    sw      $31,    KPANIC_RA*4 ($26)
    mfc0    $2,     $9
    mfc0    $3,     $8
    mfc0    $4,     $12
    mfc0    $5,     $13
    mfc0    $6,     $14
    mfhi    $7
    mflo    $8
    sw      $2,     KPANIC_COUNT*4 ($26)
    sw      $3,     KPANIC_BAR*4 ($26)
    sw      $4,     KPANIC_SR*4($26)
    sw      $5,     KPANIC_CR*4($26)
    sw      $6,     KPANIC_EPC*4($26)
    sw      $7,     KPANIC_HI*4($26)
    sw      $8,     KPANIC_LO*4($26)

    move    $4,     $26                 // get arg
    la      $26,    kdump               // kdump( KPanicRegsVal )
    j       $26                         // kdump ends with exit() and never retuns
