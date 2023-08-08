/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-25
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     platforms/virt/cpu/cache.S
  \author   Franck Wajsburt
  \brief    cpu specific assembly code which implement a part of cpu HAL

\*------------------------------------------------------------------------------------------------*/

//--------------------------------------------------------------------------------------------------
// L1 Cache operations
//--------------------------------------------------------------------------------------------------

.section .text

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
