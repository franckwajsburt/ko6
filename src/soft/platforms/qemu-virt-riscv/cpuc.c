/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-24
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     platform/virt/cpuc.c
  \author   Nolan Bled
  \brief    CPU specific c code which implement a part of hcpu.h API

\*------------------------------------------------------------------------------------------------*/

#include <platforms/qemu-virt-riscv/cpu.h>
#include <kernel/klibc.h>

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


//--------------------------------------------------------------------------------------------------
// end of kpanic() to dump all register value and threads list
//--------------------------------------------------------------------------------------------------


/** 
 * \brief   this table cannot be static because it is used by hcpua.S file
 */
char * KDumpMessage;

/** 
 * \brief   Table filled by kpanic() and read by kdump
 *          this table cannot be static because it is used by hcpua.S file
 */
unsigned KPanicRegsVal[KPANIC_REGS_NR];

/** 
 * \brief   register name, the order must the same as defined in hcpu_soc.h files
 */
static char *KPanicRegsName[KPANIC_REGS_NR] = {
    "RA ", "SP ", "GP ", "TP ", "T0 ", "T1 ", "T2 ", "S0 ",
    "S1 ", "A0 ", "A1 ", "A2 ", "A3 ", "A4 ", "A5 ", "A6 ",
    "A7 ", "S2 ", "S3 ", "S4 ", "S5 ", "S6 ", "S7 ", "S8 ",
    "S9 ", "S10 ", "S11 ", "T3 ", "T4 ", "T5 ", "T6 ",
    "MCYCLE ", "MTVAL ", "MSTATUS ", "MEPC "
};

/** 
 * \brief   cause name (see 3.1.15, privileged spec)
 */
static char *KPanicCauseName[16] = {
    [0 ... 15] = "Other cause or Application exit()",   // default value for all table case,
    [0] = "Instruction address misaligned",
    [1] = "Instruction access fault",
    [2] = "Illegal instruction",
    [3] = "Breakpoint",
    [4] = "Load address misaligned",
    [5] = "Load access fault",
    [6] = "Store/AMO address misaligned",
    [7] = "Store/AMO access fault",
    [9] = "Environment call from S-mode",
    [11] = "Environment call from M-mode",
    [12] = "Instruction page fault",
    [13] = "Load page fault",
    [15] = "Store/AMO page fault"
};

/** 
 * \brief   dump all registers values from a table filled by kpanic() then exit program
 *          this function cannot be static because it is used by hcpua.S file
 */
void kdump (unsigned cause, unsigned reg_tab[])
{
    int nl = 0;
    char * message = (KDumpMessage) ? KDumpMessage : KPanicCauseName[cause];

    kprintf ("\n[%d] <%p> KERNEL PANIC: %s\n\n",
            KPanicRegsVal[KPANIC_MCYCLE],               // TSC Time Stamp Counter
            KPanicRegsVal[KPANIC_MEPC],                 // faulty instruction address
            message);                                   // Comprehensive cause name
    for (int i = 0; i < KPANIC_REGS_NR; i++) {
        kprintf ("%s : %p\t", KPanicRegsName[i], KPanicRegsVal[i]);
        if (++nl == 4) {
            kprintf ("\n");
            nl = 0;
        }
    }
    kprintf("\n");

    sched_dump();
    while (1); // never returns
}
