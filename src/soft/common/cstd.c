/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-03
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     commmon/cstd.c
  \author   Franck Wajsburt, Nolan Bled
  \brief    Standard C functions

\*------------------------------------------------------------------------------------------------*/

#include <cstd.h>

void wzero (void *addr, size_t n)
{
    unsigned *a = addr;
    if (a)
    do { *a++ = 0; } while (n -= sizeof(unsigned));
}

void *memset (void *s, int c, unsigned n)
{
    char *p = s;

    while (n--)
        *p++ = c;
    return s;
}

void *memcpy (char *dest, const char *src, unsigned n)
{
  char *d = dest;
  while (n--)
    *d++ = *src++;
  return dest;
}

int strlen (const char *buf)
{
    int n = 0;
    if (buf) while (*buf++) n++;
    return n;
}

void *strncpy (char *dest, char *src, unsigned n)
{
    unsigned i;
    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for (; i < n; i++)
        dest[i] = '\0';
    return dest;
}

int atoi (char *val)
{
    int res = 0;
    int neg = 0;

    while ((*val == ' ') || (*val == '\t'))     // look for the first non-space character
        val++;

    if (*val == '-') {                          // if negative
        neg = 1;                                // register it
        val++;                                  // go to the first real number
    } else {
        neg = 0;                                // it is positive
    }

    while (*val) {                              // while there is a char
        res = (res * 10) + (*val - '0');        // add  to the result
        val++;
    }

    return (neg) ? -res : res;                  // return the right result
}

static char xdigit[] = "0123456789abcdef";      // hexadecimal digit
int vsnprintf (char * buffer, unsigned size, char *fmt, va_list ap)
{
    char arg[16];                               // buffer used to build the argument
    char *tmp;                                  // temporary pointer used to build arguments
    char *buf = buffer;                         // pointer to the current filling position in buffer
    int res;                                    // function result (number of printed char)
    int val;                                    // argument value
    int i;                                      // temporary variable
    int count = size - 1;                       // max nb of char in buffer (-1 because of last 0)

    while (*fmt) {                              // for all char in fmt
        while ((*fmt) && (*fmt != '%')) {       // while char exists and it is not a %
            *buf++ = *fmt++;                    // copy it in output buffer
            if (--count == 0)                   // decrement count
                goto abort;                     // and abort if there is no space anymore
        }
        if (*fmt == '%') {                      // if char is a %
            fmt++;                              // go to the next char
            switch (*fmt) {                     // study the different cases
            case '%':                           // case %%
                *buf++ = '%';                   // then it is just %
                if (--count == 0)               // go to the next char if there is space yet
                    goto abort;
                goto next;
            case 'c':                           // case %c (char)
                *buf++ = (char)va_arg (ap, int);// get the argument and copy it in output buffer
                if (--count == 0)               // go to the next char if there is space yet
                    goto abort;
                goto next;
            case 's':                           // case %s (string)
                tmp = va_arg (ap, char *);      // tmp points to this string argumment
                tmp = (tmp) ? tmp : "(null)";   // replace "" by "(null)"
                goto copy_tmp;                  // go to copy tmp in buffer
            case 'd':                           // case %d (decimal int)
                val = va_arg (ap, int);         // val <- value to convert in ascii
                i = (val == 0x80000000);        // i <- 1 if val==-(MAXINT+1) MAXINT==0x7FFFFFFF
                if (val < 0) {                  // if val is negative
                    val = -val;                 // val <- abs(val)
                    val = val - i;              // but if val==-(MAXINT+1) then val <- MAXINT
                    *buf++ = '-';               // put '-' to the output buffer
                    if (--count == 0)           // check if there is space yet
                        goto abort;
                }
                tmp = arg + sizeof (arg);       // goto at the end of tmp buffer
                *--tmp = '\0';                  // put the ending char 0
                do {
                    *--tmp = (val % 10) + '0';  // compute the unit digit
                    *tmp = *tmp + i;            // but add 1 if we had decremented val just before
                    i = 0;                      // must be done only once
                    val = val / 10;             // go to the next digit
                }
                while (val);                    // until val becomes 0
                goto copy_tmp;                  // go to copy tmp in buffer
            case 'p':                           // case %p (pointer)
            case 'x':                           // case %x (simple hexadecimal)
                val = va_arg (ap, int);         // val <- value to convert in ascii
                tmp = arg + sizeof (arg);       // goto at the end of tmp buffer
                *--tmp = '\0';                  // put the ending char 0
                i = 0;                          // i is used to count the digits
                do {
                    *--tmp = xdigit[val & 0xF]; // compute the unit digit
                    val = (unsigned) val >> 4;  // go to the next digit
                    i++;                        // digits counter
                } while (val);                  // until val becomes 0
                if (*fmt == 'p') {              // if it is a pointer
                    while (i < 8) {             // then complete with '0'
                        *--tmp = '0';
                        i++;
                    }
                }
                goto copy_tmp;                  // go to copy tmp in buffer
            default:                            // if format not recognized
                *buf++ = *fmt;                  // then just copy it in output buffer
                if (--count == 0)               // go to the next char if there is space yet
                    goto abort;
                goto next;
            }
          copy_tmp:
            while (*tmp) {                      // copy tmp in output buffer
                *buf++ = *tmp++;
                if (--count == 0)               // check if there is space yet
                    goto abort;
            }
          next:
            fmt++;                              // go to the next char
        }
    }
  abort:
    *buf = '\0';                                // put the ending char 0
    res = (int)((unsigned)buf-(unsigned)buffer);// compute the number of char to write
    return res;                                 // and return it
}

int snprintf(char *str, unsigned size, char *fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
    int res = vsnprintf(str, size, fmt, ap);
    va_end(ap);
    return res;
}


/*
 * Code from here is trash I copy/pasted in order to make libfdt work, it's only temporary 
 */

int memcmp (const void *str1, const void *str2, size_t count)
{
  register const unsigned char *s1 = (const unsigned char*)str1;
  register const unsigned char *s2 = (const unsigned char*)str2;

  while (count-- > 0)
    {
      if (*s1++ != *s2++)
	  return s1[-1] < s2[-1] ? -1 : 1;
    }
  return 0;
}

void bcopy (const void *src, void *dest, size_t len)
{
  if (dest < src)
    {
      const char *firsts = (const char *) src;
      char *firstd = (char *) dest;
      while (len--)
	*firstd++ = *firsts++;
    }
  else
    {
      const char *lasts = (const char *)src + (len-1);
      char *lastd = (char *)dest + (len-1);
      while (len--)
        *lastd-- = *lasts--;
    }
}

void *memmove (void *s1, const void *s2, size_t n)
{
  bcopy (s2, s1, n);
  return s1;
}

void *memchr (register const void *src_void, int c, size_t length)
{
  const unsigned char *src = (const unsigned char *)src_void;
  
  while (length-- > 0)
  {
    if (*src == c)
     return (void *)src;
    src++;
  }
  return NULL;
}

char *strchr (register const char *s, int c)
{
  do {
    if (*s == c)
      {
	return (char*)s;
      }
  } while (*s++);
  return (0);
}

char *strrchr (register const char *s, int c)
{
  char *rtnval = 0;

  do {
    if (*s == c)
      rtnval = (char*) s;
  } while (*s++);
  return (rtnval);
}

size_t strnlen (const char *s, size_t maxlen)
{
  size_t i;

  for (i = 0; i < maxlen; ++i)
    if (s[i] == '\0')
      break;
  return i;
}

/*
 * This one has a different copyright: https://github.com/gcc-mirror/gcc/blob/master/libiberty/strtoul.c
 */
unsigned long strtoul(const char *nptr, char **endptr, register int base)
{
	register const char *s = nptr;
	register unsigned long acc;
	register int c;
	register unsigned long cutoff;
	register int neg = 0, any, cutlim;

	/*
	 * See strtol for comments as to the logic used.
	 */
	do {
		c = *s++;
	} while (c == ' ');
	if (c == '-') {
		neg = 1;
		c = *s++;
	} else if (c == '+')
		c = *s++;
	if ((base == 0 || base == 16) &&
	    c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;
	cutoff = (unsigned long)ULONG_MAX / (unsigned long)base;
	cutlim = (unsigned long)ULONG_MAX % (unsigned long)base;
	for (acc = 0, any = 0;; c = *s++) {
		if ('0' <= c && c <= '9')
			c -= '0';
		else if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'))
			c -= ('A' <= c && c <= 'Z') ? 'A' - 10 : 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0) {
		acc = ULONG_MAX;
	} else if (neg)
		acc = -acc;
	if (endptr != 0)
		*endptr = (char *) (any ? s - 1 : nptr);
	return (acc);
}
