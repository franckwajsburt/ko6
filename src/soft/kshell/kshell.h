/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date        2025-02-24
  | / /(     )/ _ \     \copyright   2021 Sorbonne University
  |_\_\ x___x \___/                  https://opensource.org/licenses/MIT

  \file     kshell/kshell.h
  \author   Lili Zheng, Marco Leon, Franck Wajsburt
  \brief    kshell is ko6's command interpreter. 
            It's not a real process, since ko6 can only have one process at a time.
            However, as it is a kernel thread, it does not share its own data with the user app.
      
\*------------------------------------------------------------------------------------------------*/

extern void kshell (void);
