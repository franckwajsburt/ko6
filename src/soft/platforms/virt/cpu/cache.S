/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-25
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     platforms/virt/cpu/cache.S
  \author   Nolan Bled
  \brief    cpu specific assembly code which implement a part of cpu HAL

\*------------------------------------------------------------------------------------------------*/

//--------------------------------------------------------------------------------------------------
// L1 Cache operations
//--------------------------------------------------------------------------------------------------

.section .text

.globl cachelinesize // ------------------ unsigned cachelinesize (void)
cachelinesize:
    la      a0, 256     // this is arbitrary, qemu does not simulate L1 cache
    ret

.globl dcache_buf_invalidate    // ---------- void dcache_buf_invalidate (void *buf, unsigned size);
.globl dcache_invalidate        // -------------- void dcache_invalidate (void *addr);
dcache_buf_invalidate:
dcache_invalidate:
    ret                 // the is no way to invalidate dcache in riscv
                        // + qemu is assumed to have cache coherency

.globl uncached_load // ------------------ unsigned ucached_load (void * addr)
uncached_load:
    lw      a0, 0(a0)   // again, there is no way to do an uncached load in riscv
                        // but this is not important since there's no difference between
                        // loading from main memory or l1 cache in qemu
    ret

