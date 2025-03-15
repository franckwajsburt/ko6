/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-02
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     common/syscall.h
  \author   Franck Wajsburt
  \brief    Syscall numbers and system call function prototype (CPU specific)

\*------------------------------------------------------------------------------------------------*/

#ifndef _SYSCALL_H_
#define _SYSCALL_H_

//-------------------------------------- used in libc.c
#define SYSCALL_EXIT            0
#define SYSCALL_READ            1
#define SYSCALL_WRITE           2
#define SYSCALL_CLOCK           3
#define SYSCALL_DMA_MEMCPY      4
#define SYSCALL_CACHELINESIZE   5
#define SYSCALL_DCACHEBUFINVAL  6
#define SYSCALL_DCACHEINVAL     7
#define SYSCALL_SBRK            8
#define SYSCALL_ERRNO           9
//-------------------------------------- used in ulib/thread.c
#define SYSCALL_THREAD_CREATE   10
#define SYSCALL_THREAD_YIELD    11
#define SYSCALL_THREAD_EXIT     12
#define SYSCALL_SCHED_DUMP      13
#define SYSCALL_THREAD_JOIN     14
#define SYSCALL_MUTEX_INIT      15
#define SYSCALL_MUTEX_LOCK      16
#define SYSCALL_MUTEX_UNLOCK    17
#define SYSCALL_MUTEX_DESTROY   18
#define SYSCALL_BARRIER_INIT    19
#define SYSCALL_BARRIER_WAIT    20
#define SYSCALL_BARRIER_DESTROY 21
//-------------------------------------- shellsyscall
#define SYSCALL_KSHELL          22
//-------------------------------------- maximum number
#define SYSCALL_NR              32


#ifndef __DEPEND__
#if ((SYSCALL_NR != 16) && (SYSCALL_NR != 32))
#error SYSCALL_NR doit être une puissance de 2
#endif
#endif

/**
 * __ASSEMBLER__ is automatically defined when gcc runs the assembler (.S source file)
 * (see https://bit.ly/32tv7u8) thus this part is not included in assembler files (.S)
 */
#ifndef __ASSEMBLER__
/**
 * \brief   allows to enter the kernel, it is written in assembler in crt0.c
 * \param   a0 1st generic parameter
 * \param   a1 2nd generic parameter
 * \param   a2 3rd generic parameter
 * \param   a3 4th generic parameter
 * \param   syscall_code syscall code must be one of those defined above : SYSCALL_xxx
 * \return  depends on the syscall code but often it is the success information: 0 good else failure
 */
extern int syscall_fct (int a0, int a1, int a2, int a3, int syscall_code);
#endif//__ASSEMBLER__

#endif//_SYSCALL_H_
