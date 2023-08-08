#include <hal/cpu/riscv/context.h>

//--------------------------------------------------------------------------------------------------
// Thread management
//--------------------------------------------------------------------------------------------------

void thread_context_init (int context[], void * bootstrap, void * stack_pointer)
{
    /**
     * mstatus of a user-thread should have:
     *  mstatus.MPIE = 0 (we don't want interrupts when we go back in M-mode) 
     *  mstatus.MPP = M-mode (we want to go back in m-mode)
     *  mstatus.MIE = 1 (interrupts are enabled)
     */
    context[TH_CONTEXT_MSTATUS] = (3 << 11) | (1 << 3);
    context[TH_CONTEXT_RA] = (int)bootstrap;        // goto thread_bootstrap
    context[TH_CONTEXT_SP] = (int)stack_pointer;    // stack beginning 
}