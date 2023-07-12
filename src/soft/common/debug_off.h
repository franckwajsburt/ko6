/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-04
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     commmon/debug_off.h
  \author   Franck Wajsburt
  \brief    see debug_on.h

\*------------------------------------------------------------------------------------------------*/

#include <common/debug_on.h>   // def the debugging stuff, the permanent ones, then undo the temporary ones

//-------------------------------------------------------------------------------------------------

#ifdef BIP
#   undef BIP
#   undef VAR
#   undef VARN
#   undef INFO
#   undef ASSERT
#endif

#define BIP(c)
#define VAR(fmt,arg)
#define VARN(fmt,arg)
#define INFO(fmt,arg...)
#define ASSERT(cond,fmt,arg...)
