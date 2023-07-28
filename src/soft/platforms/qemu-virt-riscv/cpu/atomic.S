/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-25
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     platforms/virt/atomic.S
  \author   Nolan Bled
  \brief    cpu specific assembly code which implement a part of hcpu.h API
            - There are several independant parts :
              - spinlock & atomic operations

\*------------------------------------------------------------------------------------------------*/

//--------------------------------------------------------------------------------------------------
// spin and atomic operations
//--------------------------------------------------------------------------------------------------
.section .text

.globl spin_lock // ---------------------- void spin_lock (spinlock_t *s)
spin_lock_delay:                    // wait for t0 cycles
    addi    t0, t0, -1  
    bnez    t0, spin_lock_delay
spin_lock:
    lr.w    t1, 0(a0)               // linked-load (make a reservation)
    la      t0, 50                  
    bnez    t1, spin_lock_delay     // if the lock is still taken (value!=0)
                                    // then wait for 50 cycles before retrying
spin_lock_sc:
    li      t0, 1                   
    sc.w    t0, t0, 0(a0)           // take the lock (wite value=1)
    bnez    t0, spin_lock           // check that sc succeeded (i.e. rd=0)
    ret

.globl spin_unlock // -------------------- void spin_unlock (spinlock_t *s)
spin_unlock:
    sw      zero, 0(a0)                 // free the lock (write value=0)
    fence                               // empty the write buffer
    ret                                 // the lock is released

.globl atomic_add // --------------------- int atomic_add (int *var, int val)
atomic_inc:
    amoadd.w    t0, a1, 0(a0)           // atomic add (see spec 8.4 "Atomic Memory Operations")
    add         a0, t0, a1              // perform the addition a 2nd time to get the result
    ret
