/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-08-14
  | / /(     )/ _ \     \copyright  2021-2023 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     kernel/kprocess.h
  \author   Franck Wajsburt
  \brief    process management

\*------------------------------------------------------------------------------------------------*/


#ifndef _KPROCESS_H_
#define _KPROCESS_H_

// Maximum number of thread it can be changed
//--------------------------------------------------------------------------------------------------

#define PROCESS_THREAD_MAX          4

//--------------------------------------------------------------------------------------------------
// A process is a resource container with threads and everything needed to run them.
// As long as ko6 does not manage virtual memory, only one process is running.
//--------------------------------------------------------------------------------------------------

/**
 * \brief   Hidden definition of process_t, other C files don't know what is in the struct process_s
 */
typedef struct process_s * process_t;

#endif//_KPROCESS_H_
