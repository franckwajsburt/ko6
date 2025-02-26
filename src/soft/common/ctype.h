/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-02-26
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     commmon/ctype.h
  \author   Franck Wajsburt
  \brief    ascii service functions (see common/ctype.c for tables definitions)
             - \a : Bell (BEL)           [0x07] Produces an audible alert (bell sound)
             - \b : Backspace (BS)       [0x08] Moves cursor one position back
             - \t : Horizontal Tab (HT)  [0x09] Moves to the next horizontal tab stop
             - \n : Line Feed (LF)       [0x0A] Moves to a new line (Unix newline)
             - \v : Vertical Tab (VT)    [0x0B] Moves to the next vertical tab stop
             - \f : Form Feed (FF)       [0x0C] Moves to a new page (form feed)
             - \r : Carriage Return (CR) [0x0D] Returns cursor to the beginning of the line
             - \0 : Null character (NUL) [0x00] Marks the end of a C-style string
            
\*------------------------------------------------------------------------------------------------*/

#define _is(c,tbl)  ((tbl[(c)/32]  >> ((c)%32)) & 1)
#define isdigit(c)  _is(c,_digit)
#define isupper(c)  _is(c,_upper)
#define islower(c)  _is(c,_lower)
#define isspace(c)  _is(c,_space)
#define ispunct(c)  _is(c,_punct)
#define isxdigit(c) _is(c,_xdigit)
#define isalpha(c)  (isupper(c)   || islower(c))
#define isalnum(c)  (isdigit(c)   || isalpha(c))
#define isblank(c)  (((c) == ' ') || ((c) == '\t'))
#define iscntrl(c)  (((c) <  ' ') || ((c) == 127))
#define isgraph(c)  (((c) >  ' ') && ((c) <  127))
#define isprint(c)  (((c) >= ' ') && ((c) <  127))
#define isascii(c)  ((c) <= 127)
#define toupper(c)  ((islower(c)) ? (c)-('a'-'A') : (c))
#define tolower(c)  ((isupper(c)) ? (c)+('a'-'A') : (c))
