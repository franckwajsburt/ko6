/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-02-24
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/cpu/mips/threadc.c
  \author   Franck Wajsburt
  \brief    CPU specific c code which implement threading functions

\*------------------------------------------------------------------------------------------------*/

#include <hal/cpu/mips/context.h>

//--------------------------------------------------------------------------------------------------
// Thread management
//--------------------------------------------------------------------------------------------------

void thread_context_init (int context[], void * bootstrap, void * stack_pointer)
{
    context[TH_CONTEXT_SR] = 0x413;                 // HWI0=1 UM=1 EXL=1 IE=1
    context[TH_CONTEXT_RA] = (int)bootstrap;        // goto thread_bootstrap
    context[TH_CONTEXT_SP] = (int)stack_pointer;    // stack beginning 
}

void kthread_context_init (int context[], void * bootstrap, void * stack_pointer)
{
    context[TH_CONTEXT_SR] = 0x403;                 // HWI0=1 UM=0 EXL=1 IE=1
    context[TH_CONTEXT_RA] = (int)bootstrap;        // goto thread_bootstrap
    context[TH_CONTEXT_SP] = (int)stack_pointer;    // stack beginning 
}
