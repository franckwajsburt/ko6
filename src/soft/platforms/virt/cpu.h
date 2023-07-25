/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-24
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     platform/virt/cpu.h
  \author   Nolan Bled
  \brief    CPU specific defines

\*------------------------------------------------------------------------------------------------*/

#ifndef _CPU_H_
#define _CPU_H_


//--------------------------------------------------------------------------------------------------
// Thread context table's indexes
// - TH_CONTEXT_regname is the index in context[] array used to save regname register
//   for exemple TH_CONTEXT_S0==1 (S0 is the $16 register) means thread->context[1] is used for $16
// - TH_TID is used to read tid in thread_load, because tracelog needs to know which thread starts
//   in the thread structure TID FIELD IS PLACED JUST BEFORE CONTEXT[] then context[-1]==tid
//   of course, it is not possible to write context[-1] in C, but it is possible in assembler
//   look at kernel/hcpua.S function thread_load() to see where TH_TID is used
//-------------------------------------------------------------------------------------------------

#define TH_TID             -1       /* see comment above: TID is just below the CONTEXT table */
#define TH_CONTEXT_S0       0
#define TH_CONTEXT_S1       1
#define TH_CONTEXT_S2       2
#define TH_CONTEXT_S3       3
#define TH_CONTEXT_S4       4
#define TH_CONTEXT_S5       5
#define TH_CONTEXT_S6       6
#define TH_CONTEXT_S7       7
#define TH_CONTEXT_S8       8
#define TH_CONTEXT_S9       9
#define TH_CONTEXT_S10     10
#define TH_CONTEXT_S11     11
#define TH_CONTEXT_MSTATUS 12
#define TH_CONTEXT_MEPC    13
#define TH_CONTEXT_RA      14
#define TH_CONTEXT_SP      15
#define TH_CONTEXT_SIZE    16

//--------------------------------------------------------------------------------------------------
// register dump table's indexes used by kpanic() and kdum()
//--------------------------------------------------------------------------------------------------

#define KPANIC_RA        0
#define KPANIC_SP        1
#define KPANIC_GP        2
#define KPANIC_TP        3
#define KPANIC_T0        4
#define KPANIC_T1        5
#define KPANIC_T2        6
#define KPANIC_S0        7
#define KPANIC_S1        8
#define KPANIC_A0        9
#define KPANIC_A1        10
#define KPANIC_A2        11
#define KPANIC_A3        12
#define KPANIC_A4        13
#define KPANIC_A5        14
#define KPANIC_A6        15
#define KPANIC_A7        16
#define KPANIC_S2        17
#define KPANIC_S3        18
#define KPANIC_S4        19
#define KPANIC_S5        20
#define KPANIC_S6        21
#define KPANIC_S7        22
#define KPANIC_S8        23
#define KPANIC_S9        24
#define KPANIC_S10       25
#define KPANIC_S11       26
#define KPANIC_T3        27
#define KPANIC_T4        28
#define KPANIC_T5        29
#define KPANIC_T6        30
#define KPANIC_MCYCLE    31
#define KPANIC_MTVAL     33
#define KPANIC_MSTATUS   34
#define KPANIC_MEPC      35
#define KPANIC_REGS_NR   36

#endif//_CPU_H_
