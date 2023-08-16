/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-03
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     kernel/ksyscall.h
  \author   Franck Wajsburt
  \brief    definition of the syscall vector and some functions, 
            the other syscalls are in more specific files (kthread.c, ksynchro.c, etc.).

\*------------------------------------------------------------------------------------------------*/

#include <hal/devices/dma.h>
#include <kernel/klibc.h>

static int unknown_syscall (int a0, int a1, int a2, int a3, int syscall_code)
{
    kprintf ("Unknown Syscall : %d\n", syscall_code);
    kprintf ("a0 : 0x%p (%d)\n", a0, a0);
    kprintf ("a1 : 0x%p (%d)\n", a1, a1);
    kprintf ("a2 : 0x%p (%d)\n", a2, a2);
    kprintf ("a3 : 0x%p (%d)\n", a3, a3);
    return ENOSYS;
}

// FIXME dma_memcpy_user address checking could be more specific to stay in .data section
static void * dma_memcpy_user (int * dest, int * src, size_t n)
{
    if ((unsigned)src >= 0x80000000) return NULL;
    if ((unsigned)dest >= 0x80000000) return NULL;
    if ((unsigned)src+n >= 0x80000000) return NULL;
    if ((unsigned)dest+n >= 0x80000000) return NULL;
    if (n >= 0x80000000) return NULL;

    // Get the DMA device
    struct dma_s *dma = dma_get(0);
    if (!dma)
        // If no DMA is available, do a reguler memcpy
        // TODO: verify this isn't dangerous
        return memcpy((char*) dest, (char*) src, n * 4);
    else
        return dma->ops->dma_memcpy (dma, dest, src, n);
}

static int dcache_buf_inval_user (void * buf, size_t size)
{
    if ((unsigned)buf >= 0x80000000) return EPERM;
    if ((unsigned)buf+size >= 0x80000000) return EPERM;
    dcache_buf_invalidate (buf, size);
    return SUCCESS;
}

static int dcache_inval_user (void * addr)
{
    if ((unsigned)addr >= 0x80000000) return EPERM;
    dcache_invalidate (addr);
    return SUCCESS;
}

void *SyscallVector[] = {
    [0 ... SYSCALL_NR - 1   ] = unknown_syscall,   /* default function */
    [SYSCALL_EXIT           ] = exit,
    [SYSCALL_READ           ] = tty_read,
    [SYSCALL_WRITE          ] = tty_write,
    [SYSCALL_CLOCK          ] = clock,
    [SYSCALL_DMA_MEMCPY     ] = dma_memcpy_user,
    [SYSCALL_CACHELINESIZE  ] = cachelinesize,
    [SYSCALL_DCACHEBUFINVAL ] = dcache_buf_inval_user,
    [SYSCALL_DCACHEINVAL    ] = dcache_inval_user,
    [SYSCALL_SBRK           ] = sbrk,
    //[SYSCALL_ERRNO          ] = __errno_location,
    [SYSCALL_THREAD_CREATE  ] = thread_create,
    [SYSCALL_THREAD_YIELD   ] = thread_yield,
    [SYSCALL_THREAD_EXIT    ] = thread_exit,
    [SYSCALL_SCHED_DUMP     ] = sched_dump,
    [SYSCALL_THREAD_JOIN    ] = thread_join,
    [SYSCALL_MUTEX_INIT     ] = thread_mutex_init,
    [SYSCALL_MUTEX_LOCK     ] = thread_mutex_lock,
    [SYSCALL_MUTEX_UNLOCK   ] = thread_mutex_unlock,
    [SYSCALL_MUTEX_DESTROY  ] = thread_mutex_destroy,
    [SYSCALL_BARRIER_INIT   ] = thread_barrier_init,
    [SYSCALL_BARRIER_WAIT   ] = thread_barrier_wait,
    [SYSCALL_BARRIER_DESTROY] = thread_barrier_destroy,
};
