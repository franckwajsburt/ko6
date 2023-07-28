/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-03
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     commmon/cstd.c
  \author   Franck Wajsburt, Nolan Bled
  \brief    Standard C functions

\*------------------------------------------------------------------------------------------------*/

#include <common/cstd.h>

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

int memcmp (const void *str1, const void *str2, size_t n)
{
    const unsigned char *s1 = (const unsigned char*)str1;
    const unsigned char *s2 = (const unsigned char*)str2;

    while (n-- > 0) {
        if (*s1++ != *s2++)
            return s1[-1] < s2[-1] ? -1 : 1;
    }
    
    return 0;
}

void *memmove (void *dest, const void *src, size_t n)
{
    if (dest < src) {
        const char *firsts = (const char *) src;
        char *firstd = (char *) dest;
        while (n--)
	        *firstd++ = *firsts++;
    } else {
        const char *lasts = (const char *)src + (n-1);
        char *lastd = (char *)dest + (n-1);
        while (n--)
            *lastd-- = *lasts--;
    }

    return dest;
}

void *memchr (const void *src, int c, size_t n)
{
    const unsigned char *csrc = (const unsigned char *)src;
  
    while (n-- > 0) {
        if (*csrc == c)
            return (void *)csrc;
        csrc++;
    }

    return NULL;
}

int strlen (const char *buf)
{
    int n = 0;
    if (buf) while (*buf++) n++;
    return n;
}

size_t strnlen (const char *s, size_t n)
{
    size_t i;

    for (i = 0; i < n; ++i)
        if (s[i] == '\0')
            break;
    
    return i;
}

char *strchr (const char *s, int c)
{
    do {
        if (*s == c) {
	        return (char*)s;
        }
    } while (*s++);
  
    return (0);
}

char *strrchr (const char *s, int c)
{
    char *rtnval = 0;

    do {
        if (*s == c)
            rtnval = (char*) s;
    } while (*s++);
  
    return (rtnval);
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

int strcmp (const char *s1, const char *s2)
{
    unsigned char c1, c2;
    do {
        c1 = (unsigned char) *s1++;
        c2 = (unsigned char) *s2++;
        if (c1 == 0)
            return c1 - c2;
    } while (c1 == c2);

    return c1 - c2;
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

// https://cplusplus.com/reference/cstdlib/strtol/
unsigned long int strtoul(char *s, char **endptr, int base)
{
    while (*s == ' ' 
        || *s == '\t' 
        || *s == '\n'
        || *s == '\v'
        || *s == '\f'
        || *s == '\r')                              // Skip the whitespaces
        s++;

    if (*s == '+')
        s++;

    if (!base) {                                    // If base is zero, 
                                                    // try to guess the base from the format 
        if (*s == '0') {
            s++;
            if (*s == 'x' || *s == 'X') {           // 0x, 0X prefix means hexadecimal
                s++;
                base = 16;
            } else if ('0' <= *s && *s <= '9') {    // 0d with d a digit means octal
                base = 8;
            }
        } else if ('0' <= *s && *s <= '9') {        // We found a digit, let's guess decimal
            base = 10;
        }
    }

    if (!base) {                                    // If after the guessing block we still 
                                                    // don't know what the base is, abort
        if (endptr) *endptr = s;
        return 0;
    }

    unsigned long n = 0;
    char overflow = 0;
    char c;
    while (*s) {
        c = *s;
        if ('0' <= c && c <= '9') {                 // Convert a char digit to its value
            c -= '0';
        } else if ('a' <= c && c <= 'z') {          // Convert a char to its value 
                                                    // (A = a = 10, Z = z = 36)
            c -= 'a';
            c += 10;
        } else if ('A' <= c && c <= 'Z') {
            c -= 'A';
            c += 10;
        } else {
            break;
        }

        if (c >= base)                              // If the digit is outside the range of 
                                                    // the base (ex: g is outside hexadecimal
                                                    // range) the number is not decodable
            break;
        
        if (n < ULONG_MAX / base &&                 // Check that we can multiply with base
            n * base < ULONG_MAX - c) {             // and add char without overflow
            n *= base;
            n += c;
        } else {
            overflow = 1;
            break;
        }

        s++;
    }

    if (endptr) *endptr = s;
    if (overflow)
        return 0;

    return n;
}

