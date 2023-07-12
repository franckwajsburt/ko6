/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-02
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/almo1/hcpua.S
  \author   Franck Wajsburt
  \brief    cpu specific assembly code which implement a part of hcpu.h API
            - There are several independant parts :
              - boot code 
              - kentry, the uniq entry of the kernel (but kinit at the very beginning)
              - the 3 events handlers : syscall ; irq ; exception
              - special registers access 
              - thread context operations
              - L1 operations
              - irq mask operations 
              - spinlock & atomic operations

\*------------------------------------------------------------------------------------------------*/

#include <platforms/mips/almo1/cpu.h>
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


//--------------------------------------------------------------------------------------------------
// Special registers
//--------------------------------------------------------------------------------------------------


.globl clock // -------------------------- int clock (void) is an external function
clock:                                  // clock is a terminal function then don't use the stack
    mfc0    $2,     $9                  // 32 bits counter in the coprocessor system (c0_$9)
    jr      $31                         // get it in $2 and return

.globl cpuid // -------------------------- int cpuid (void)
cpuid:
    mfc0    $2,     $15,    1           // the cpu number is the coprocessor system (c0_$15,1)
    andi    $2,     $2,     0x3FF       // only 11 bits --> 2048 cpu max :-)
    jr      $31                         // get it in $2 and return


//--------------------------------------------------------------------------------------------------
// Thread management
//--------------------------------------------------------------------------------------------------


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


//--------------------------------------------------------------------------------------------------
// L1 Cache operations
//--------------------------------------------------------------------------------------------------


.globl cachelinesize // ------------------ unsigned cachelinesize (void)
cachelinesize:
    mfc0    $6,     $16,    1           // $16,1 =conf reg. si DL=bits(12:10) dcache_line_size=2<<DL
    ext     $2,     $6,     10,     3   // $2 = DL bits(12:10) of config register $16,1
    li      $6,     2                   // 0x2
    beqz    $2,     cachelinesize_end   // $2==0 means no cache
    sll     $2,     $6,     $2          // dcache_line_size = $6 <-- 2 << DL
cachelinesize_end:
    jr      $31                         // the delayed slot at the end of dcache_invalidate

//https://s3-eu-west-1.amazonaws.com/downloads-mips/documents/MD00090-2B-MIPS32PRA-AFP-06.02.pdf 238
//https://www.cs.cornell.edu/courses/cs3410/2008fa/MIPS_Vol2.pdf p85
.globl dcache_buf_invalidate // ---------- void dcache_buf_invalidate (void *buf, unsigned size);
.globl dcache_invalidate // -------------- void dcache_invalidate (void *addr);
dcache_buf_invalidate:
    mfc0    $6,     $16,    1           // $16,1 =conf reg. si DL=bits(12:10) dcache_line_size=2<<DL
    ext     $2,     $6,     10,     3   // $2 = DL bits(12:10) of config register $16,1
    li      $6,     2                   // 0x2
    sll     $6,     $6,     $2          // dcache_line_size = $6 <-- 2 << DL
    beqz    $2,     dcache_inval_end    // $2==0 means no cache
dcache_inval_loop:
    cache   17,     0($4)               // 17 == L1-D cache invalidate
    subu    $5,     $5,     $6          // size   -= dcache_line_size
    addu    $4,     $4,     $6          // buffer += dcache_line_size
    bgtz    $5,     dcache_inval_loop   // loop if size > 0
dcache_invalidate:
    cache   17,     0($4)               // invalidate the last line
    jr      $31
dcache_inval_end:                       // do not erase these lines, it is to use
    jr      $31                         // the delayed slot at the end of dcache_invalidate

.globl uncached_load // ------------------ unsigned ucached_load (void * addr)
uncached_load:
    ll      $2,     0($4)               // linked load is an UNCACHED LOAD
    jr	    $31


//--------------------------------------------------------------------------------------------------
// IRQ mask operations
//--------------------------------------------------------------------------------------------------


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

.globl irq_restore// -------------------- void irq_restore(unsigned)
irq_restore:
    mtc0    $4,     $12                 // SR <-- a previous SR
    jr      $31


//--------------------------------------------------------------------------------------------------
// spin and atomic operations
//--------------------------------------------------------------------------------------------------


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
