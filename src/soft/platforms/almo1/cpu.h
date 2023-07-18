/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-03
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     platform/almo1/cpu.h
  \author   Franck Wajsburt
  \brief    CPU specific defines

\*------------------------------------------------------------------------------------------------*/

#ifndef _HCPU_SOC_H_
#define _HCPU_SOC_H_


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
#define TH_CONTEXT_SR       9
#define TH_CONTEXT_EPC      10
#define TH_CONTEXT_RA       11
#define TH_CONTEXT_SP       12
#define TH_CONTEXT_SIZE     13

//--------------------------------------------------------------------------------------------------
// register dump table's indexes used by kpanic() and kdum()
//--------------------------------------------------------------------------------------------------

#define KPANIC_CR        0
#define KPANIC_AT        1
#define KPANIC_V0        2
#define KPANIC_V1        3
#define KPANIC_A0        4
#define KPANIC_A1        5
#define KPANIC_A2        6
#define KPANIC_A3        7
#define KPANIC_T0        8
#define KPANIC_T1        9
#define KPANIC_T2        10
#define KPANIC_T3        11
#define KPANIC_T4        12
#define KPANIC_T5        13
#define KPANIC_T6        14
#define KPANIC_T7        15
#define KPANIC_S0        16
#define KPANIC_S1        17
#define KPANIC_S2        18
#define KPANIC_S3        19
#define KPANIC_S4        20
#define KPANIC_S5        21
#define KPANIC_S6        22
#define KPANIC_S7        23
#define KPANIC_T8        24
#define KPANIC_T9        25
#define KPANIC_HI        26
#define KPANIC_LO        27
#define KPANIC_GP        28
#define KPANIC_SP        29
#define KPANIC_FP        30
#define KPANIC_RA        31
#define KPANIC_COUNT     32
#define KPANIC_BAR       33
#define KPANIC_SR        34
#define KPANIC_EPC       35
#define KPANIC_REGS_NR   36

#endif//_HCPU_SOC_H_
