/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-03
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/cpu/mips/kpanicc.c
  \author   Franck Wajsburt
  \brief    CPU implementation of kdump()

\*------------------------------------------------------------------------------------------------*/

#include <hal/cpu/mips/context.h>
#include <kernel/klibc.h>

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
    "CR ", "AT ", "V0 ", "V1 ", "A0 ", "A1 ", "A2 ", "A3 ",
    "T0 ", "T1 ", "T2 ", "T3 ", "T4 ", "T5 ", "T6 ", "T7 ",
    "S0 ", "S1 ", "S2 ", "S3 ", "S4 ", "S5 ", "S6 ", "S7 ",
    "T8 ", "T9 ", "HI ", "LO ", "GP ", "SP ", "FP ", "RA ",
    "TSC", "BAR", "SR ", "EPC"
};

/** 
 * \brief   cause name
 */
static char *KPanicCauseName[16] = {
    [0 ... 15] = "Other cause or Application exit()",   // default value for all table case,
    [4] = "ADEL: Illegal load address",
    [5] = "ADES: Illegal store address",
    [6] = "IBE:  Segmentation fault for intruction",
    [7] = "DBE:  Segmentation fault for data",
    [10] = "RI:   Illegal instruction",
    [11] = "CPU:  coprocessor unreachable",
    [12] = "OVF:  Overflow",
    [13] = "DIV:  Division by 0",                       // not in MIPS specification
};

/** 
 * \brief   dump all registers values from a table filled by kpanic() then exit program
 *          this function cannot be static because it is used by hcpua.S file
 */
void kdump (unsigned reg_tab[])
{
    int nl = 0;
    int cause = (KPanicRegsVal[0] >> 2) & 0xF;

    kprintf ("\n[%d] <%p> KERNEL PANIC: %s\n\n",
            KPanicRegsVal[KPANIC_COUNT],                // TSC Time Stamp Counter
            KPanicRegsVal[KPANIC_EPC],                  // faulty instruction address
            KPanicCauseName[cause]);                    // Comprehensive cause name

    for (int i = 0; i < KPANIC_REGS_NR; i++) {
        kprintf ("%s : %p     ", KPanicRegsName[i], KPanicRegsVal[i]);
        if (++nl == 4) {
            kprintf ("\n");
            nl = 0;
        }
    }
    sched_dump();
    while (1); // never returns
}
