/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-02
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     common/debug_on.h
  \author   Franck Wajsburt
  \brief    Debug messages
            Permanent
            - PANIC_IF( cond, fmt, arg...) to stop the execution in case of a fatal condition
            Temporary
            - Macros BIP(), VAR(), VARN(), INFO() & ASSERT() adds temporary debug messages
            - This is useful to find and fix errors, 
              but we don't want to display messages if there is no bugs
            - Add #include <degug_on.h> before the messages you want to display 
              so that the BIP(), VAR(), VARN(), INFO() and ASSERT() macros produce messages.
            - Add #include <degug_off.h> to stop messages from BIP(), INFO() and so on.
            - It is possible to include these files several times per c file

\*------------------------------------------------------------------------------------------------*/

#ifndef _DEBUG_ON_H_
#define _DEBUG_ON_H_

#define _FMT_(T,F,C) "[%d:%d:%s/%s] "T" (%s) "F"\n",cpuid(),clock(),__FILE__,__func__,C

/**
 * \brief   Check a condition and write a formated message on console if false and stop execution
 * \param   cond Condition to write the message and to stop and to stop
 * \param   fmt Format of printed string
 * \param   arg Variadic aguments used to form the string
 * \details Same written message than WARNING, but write PANIC then call kpanic()
 */
#ifdef _KERNEL_
#define PANIC_IF(cond,fmt,arg...) if (cond) {kprintf(_FMT_("PANIC",fmt,#cond),##arg);kpanic();}
#else
#define PANIC_IF(cond,fmt,arg...) if (cond) {fprintf(0,_FMT_("PANIC",fmt,#cond),##arg);kpanic();}
#endif//_KERNEL_

#endif//_DEBUG_

//-------------------------------------------------------------------------------------------------

#ifdef BIP
#   undef BIP
#   undef VAR
#   undef VARN
#   undef INFO
#   undef ASSERT
#endif

/**
 * \brief   Write a single char
 * \param   c char
 */
#ifdef _KERNEL_
#define BIP(c)  tty_putc(0,c)
#else
#define BIP(c)  fputc(0,c)
#endif//_KERNEL_

/**
 * \brief   Write a value of a variable
 * \param   fmt type i.e. %d, %p, etc.
 * \param   arg Variadic aguments used to form the string
 */
#ifdef _KERNEL_
#define VAR(fmt,var) kprintf("-- %s: "#var"\t "#fmt,__func__,var)
#define VARN(fmt,var) kprintf("-- %s: "#var"\t "#fmt,var)
#else
#define VAR(fmt,var) fprintf(0,"-- %s: "#var"\t "#fmt,__func__,var)
#define VARN(fmt,var) fprintf(0,"-- %s: "#var"\t "#fmt,var)
#endif//_KERNEL_

/**
 * \brief   Write a formated message on console
 * \param   fmt Format of printed string
 * \param   arg Variadic aguments used to form the string
 */
#ifdef _KERNEL_
#define INFO(fmt,arg...) kprintf(_FMT_("INFO",fmt,""),##arg)
#else
#define INFO(fmt,arg...) fprintf(0,_FMT_("INFO",fmt,""),##arg)
#endif//_KERNEL_

/**
 * \brief   Check a condition and write a formated message on console if false
 * \param   cond Condition to NOT write the message
 * \param   fmt Format of printed string
 * \param   arg Variadic aguments used to form the string
 *          example ASSERT( (var<10), "Var=%d must be < 10", var)
 *                  write nothing if var in inferior to 10
 *                  write [cpuid:time:CSourceFile:Function] ASSERT (var<10) Var=%d must be < 10
 */
#ifdef _KERNEL_
#define ASSERT(cond,fmt,arg...) if (!(cond)) kprintf(_FMT_("ASSERT",fmt,#cond),##arg)
#else
#define ASSERT(cond,fmt,arg...) if (!(cond)) fprintf(0,_FMT_("ASSERT",fmt,#cond),##arg)
#endif//_KERNEL_
