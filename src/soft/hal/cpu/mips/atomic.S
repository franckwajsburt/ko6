/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-25
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/cpu/mips/atomic.S
  \author   Franck Wajsburt
  \brief    cpu specific assembly code which implement a part of hcpu.h API
            - There are several independant parts :
              - spinlock & atomic operations

\*------------------------------------------------------------------------------------------------*/

//--------------------------------------------------------------------------------------------------
// spin and atomic operations
//--------------------------------------------------------------------------------------------------
.section .text

.globl spin_lock // ---------------------- void spin_lock (spinlock_t *s)
spin_lock_delay:
    addiu   $8,     $8,     -1          // wait $8 cycles -1
    bnez    $8,     spin_lock_delay     // if != 0 then wait again
spin_lock:
    ll      $2,     0($4)               // linked load the lock (UNCACHED LOAD)
    li      $8,     50                  // prepare delay counter
    bne     $2,     $0, spin_lock_delay // if lock==1 then try again
spin_lock_sc:
    li      $2,     1                   // try to store 1
    sc      $2,     ($4)                // at the lock address
    beqz    $2,     spin_lock           // 1 on success, 0 on fealure in that case try again
    jr      $31                         // the lock is taken

.globl spin_unlock // -------------------- void spin_unlock (spinlock_t *s)
spin_unlock:
    sw      $0,     0($4)               // free the lock
    sync                                // empty the write buffer
    jr      $31                         // the lock is released

.globl atomic_add // --------------------- int atomic_add (int *var, int val)
atomic_inc:
    ll      $2,     0($4)               // linked load the counter (UNCACHED LOAD)
    addu    $8,     $2,     $5          // $8 = counter + val
    move    $2,     $8                  // $2 is the new value to store and to return
    sc      $8,     0($4)               // try to store the new value
    beqz    $8,     atomic_inc          // 1 on success, 0 on fealure in that case try again
    jr      $31                         // the new counter value is returned in $2
