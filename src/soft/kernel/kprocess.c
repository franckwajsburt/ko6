/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-08-14
  | / /(     )/ _ \     \copyright  2021-2023 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     kernel/kprocess.c
  \author   Franck Wajsburt
  \brief    process management

\*------------------------------------------------------------------------------------------------*/

#include <klibc.h>

typedef struct process_s * process_t;

//--------------------------------------------------------------------------------------------------
// Type declarations with mandatory external accessor functions 
// When the declarations are static, that is because, they are only used in this file
//--------------------------------------------------------------------------------------------------

/**
 * \brief   process_s structure which contains all we need to run a process
 */
struct process_s {
// Thread & Scheduler : ThreadTab ; ThreadCurrentIdx ; ThreadCurrent
};
