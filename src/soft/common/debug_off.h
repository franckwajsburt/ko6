/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-18
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     common/debug_off.h
  \author   Franck Wajsburt
  \brief    see debug_on.h

\*------------------------------------------------------------------------------------------------*/

#include <common/debug_on.h>   // def the debugging stuff, then undo the temporary ones

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

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
