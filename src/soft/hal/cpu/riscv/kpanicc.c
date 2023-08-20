/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-02
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/cpu/mips/kpanicc.c
  \author   Nolan Bled
  \brief    cpu specific assembly code which implement kpanic()

\*------------------------------------------------------------------------------------------------*/

#include <hal/cpu/riscv/context.h>
#include <kernel/klibc.h>
// #include <kernel/kthread.h>

//--------------------------------------------------------------------------------------------------
// end of kpanic() to dump all register value and threads list
//--------------------------------------------------------------------------------------------------

/** 
 * \brief   Table filled by kpanic() and read by kdump
 *          this table cannot be static because it is used by kpanica.S file
 */
unsigned KPanicRegsVal[KPANIC_REGS_NR];

/** 
 * \brief   register name, the order must the same as defined in hcpu_soc.h files
 */
static char *KPanicRegsName[KPANIC_REGS_NR] = {
    "RA  ", "SP  ", "GP  ", "TP  ", "T0  ", "T1  ", "T2  ", "S0  ",
    "S1  ", "A0  ", "A1  ", "A2  ", "A3  ", "A4  ", "A5  ", "A6  ",
    "A7  ", "S2  ", "S3  ", "S4  ", "S5  ", "S6  ", "S7  ", "S8  ",
    "S9  ", "S10 ", "S11 ", "T3  ", "T4  ", "T5  ", "T6  ",      
    "MEPC", "MTVAL ", "    MSTATUS ", "   MCYCLE " 
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

    kprintf ("\n[%d] <%p> KERNEL PANIC: %s\n\n",
            0,                                          // FIXME: TSC Time Stamp Counter
            KPanicRegsVal[KPANIC_MEPC],                 // faulty instruction address
            KPanicCauseName[cause]);                    // Comprehensive cause name

    for (int i = 0; i < KPANIC_REGS_NR; i++) {
        kprintf ("%s: %p  ", KPanicRegsName[i], KPanicRegsVal[i]);
        if (++nl == 4) {
            kprintf ("\n");
            nl = 0;
        }
    }
    kprintf("\n");

    sched_dump();
    while (1); // never returns
}
