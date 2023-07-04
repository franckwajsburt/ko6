/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-03
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     commmon/cstd.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Standard C functions

\*------------------------------------------------------------------------------------------------*/

#ifndef _CSTD_H_
#define _CSTD_H_

#ifndef __DEPEND__      // this condition allows to not include stdarg.h when makedepend is used
#include <stdarg.h>     // gcc's builtin include to use variadic function (https://bit.ly/3hLXjyC)
#include <stddef.h>     // gcc's builtin include with NULL, size_t, (https://bit.ly/3lBw3p6)
#endif//__DEPEND__

#define false   0
#define true    (!false)

/**
 * \brief     set every byte in a buffer to a same value
 * \param     dest buffer address
 * \param     c value to write
 * \param     n number of bytes to write
*/
extern void *memset (void *s, int c, unsigned n);

/**
 * \brief     write 0 in a buffer 
 * \param     addr address word aligned
 * \param     n  number of bytes to erase must be a multiple of sizeof word
 */
extern void wzero (void *addr, size_t n);

/**
 * \brief     copies buffer src to the buffer dest (the buffers must be disjoints)
 * \param     dest destination buffer
 * \param     src  source buffer
 * \param     n  number of bytes to copy
 * \return    the dest buffer address
 */
extern void *memcpy (char *dest, const char *src, unsigned n);

/**
 * \brief     copies the string src, included the ending byte '\0', to the buffer dest
 * \param     dest buffer where the source string must be copied
 * \param     src  the source string
 * \param     n    size of the dest buffer
 * \return    the dest buffer address
 */
extern void *strncpy (char *dest, char *src, unsigned n);

/**
 * \brief     calculates the length of the string s, excluding the terminating null byte
 * \return    the number of characters
 */
extern int strlen (const char *buf);

/**
 * \brief     ascii to integer
 *            the number can be negative, the space at beginning are ignored
 * \param     val   string to translate, with a decimal number
 * \return    the integer corresponding to val
 */
extern int atoi (char *val);

/**
 * \brief     write a formated string to the str buffer
 *            this a simplified version which handles only: %c, %s, $d, %x and %p
 * \param     str   buffer of chars where string is formed
 * \param     size  size of buffer
 * \param     fmt   formated string
 * \param     ap    variadic arguments, i.e. variable number of arguments
 * \return    number of printed char
 */
int vsnprintf (char * buffer, unsigned size, char *fmt, va_list ap);

/**
 * \brief     write a formated string to the str buffer
 *            this a simplified version which handles only: %c, %s, $d, %x and %p
 * \param     str   buffer of chars where string is formed
 * \param     size  size of buffer
 * \param     fmt   formated string
 * \param     ...   variadic arguments, i.e. variable number of arguments
 * \return    number of printed char
 */
extern int snprintf (char *buffer, unsigned size, char *fmt, ...);

/*
 * Code from here is trash I copy/pasted in order to make libfdt work, it's only temporary 
 */
#define INT_MAX		((int)(~0U >> 1))
#define INT32_MAX	INT_MAX
#define UINT_MAX	((unsigned int)~0U)
#define UINT32_MAX	UINT_MAX
#define ULONG_MAX 	UINT_MAX

extern int memcmp (const void *str1, const void *str2, size_t count);
extern void bcopy (const void *src, void *dest, size_t len);
extern void *memmove (void *s1, const void *s2, size_t n);
extern void *memchr (register const void *src_void, int c, size_t length);
extern char *strchr (register const char *s, int c);
extern char *strrchr (register const char *s, int c);
extern size_t strnlen (const char *s, size_t maxlen);
extern unsigned long strtoul(const char *nptr, char **endptr, register int base);

#endif