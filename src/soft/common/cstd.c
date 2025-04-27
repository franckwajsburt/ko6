/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-27
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     commmon/cstd.c
  \author   Franck Wajsburt, Nolan Bled
  \brief    Standard C functions without syscalls, thus usable by the kernel and user applications.

\*------------------------------------------------------------------------------------------------*/

#include <common/cstd.h>
#include <common/esc_code.h>

#define Y       EC_BOLD EC_WHITE"'"EC_YELLOW"v"EC_WHITE"'"EC_RESET EC_CYAN
#define X       EC_ORANGE"x"EC_CYAN
#define X___X   " " X "___" X " "
char Banner_ko6[] =          // banner's text defined on several lines
EC_WHITE
"   _   "  EC_CYAN"  ___  "  EC_WHITE"  __ \n"
"  | |__"  EC_CYAN" /"Y"\\ " EC_WHITE" / /    " KO6VER "\n"
"  | / /"  EC_CYAN"(     )"  EC_WHITE"/ _ \\   SPDX-License-Identifier: MIT\n"
"  |_\\_\\"EC_CYAN  X___X    EC_WHITE"\\___/   Copyright 2021 Sorbonne University\n\n"
EC_RESET;

void *memset(void *s, int c, size_t n) {
    unsigned char *p = s;
    unsigned long clong = (unsigned char)c;
    clong |= clong << 8;
    clong |= clong << 16;                                   // Build a 32-bit constant
#ifdef __LP64__
    clong |= clong << 32;                                   // 64-bit systems
#endif

    while ((unsigned long)p % sizeof(unsigned long) && n) { // Align address to unsigned long
        *p++ = c;
        n--;
    }
    unsigned long *pword = (unsigned long *)p;              // Fill by unsigned long word
    while (n >= sizeof(unsigned long)) {
        *pword++ = clong;
        n -= sizeof(unsigned long);
    }
    p = (unsigned char *)pword;                             // Fill remaining bytes
    while (n--) {
        *p++ = c;
    }
    return s;
}

void *memcpy (void *dest, const void *src, size_t n)
{
    char *d = dest;
    const char *s = src;

    if (((unsigned long)d % sizeof(long)) ==                // if dest & src have the same alignment
        ((unsigned long)s % sizeof(long))) 
    { 
        while (((unsigned long)d % sizeof(long)) && n) {    // copy all char until first long word
            *d++ = *s++;
            n--;
        }
        long *dl = (long *)d;                               // then copy per long word
        const long *sl = (const long *)s;
        while (n >= sizeof(long)) {
            *dl++ = *sl++;
            n -= sizeof(long);
        }
        d = (char *)dl;                                     // retrieve the last words addresses
        s = (const char *)sl;
    }
    while (n--) {                                           // copy the remaing chars
        *d++ = *s++;
    }
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

int strncmp (const char *s1, const char *s2, unsigned n)
{
    unsigned char c1, c2;
    if (n == 0) return 0;
    do {
        c1 = (unsigned char) *s1++;
        c2 = (unsigned char) *s2++;
        if (c1 == 0)
            return c1 - c2;
    } while ((c1 == c2) && (--n));
    return c1 - c2;
}

int strsplit(char *str, const char *delim, char *tokens[], int maxtoken)
{
    int count = 0;
    while (*str && strchr (delim, *str)) str++;
    while (*str && (count < maxtoken)) {
        tokens[count++] = str;
        while (*str && !strchr (delim, *str)) str++;
        if (*str != '\0') {
            *str++ = '\0';
            while (strchr (delim, *str)) str++;
        }
    }
    return count;
}

char *itoa(char buffer[34], int val, int base) 
{
    if (base != 2 && base != 10 && base != 16) return NULL;

    char *ptr = buffer + 33; 
    *ptr = '\0'; 

    int is_negative = (val < 0 && base == 10); 
    if (is_negative) val = -val;

    do {
        int digit = val % base;
        *--ptr = (digit < 10) ? digit + '0' : digit - 10 + 'A'; 
        val /= base;
    } while (val > 0);

    if (is_negative) *--ptr = '-'; 

    return ptr; 
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
    /*long val64_lsb;
    lo ng val64_msb;
    long long val64;
    */
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
            /* case 'l':                           // TODO: handle u,d formats
                fmt++;
                switch (*fmt) {
                case 'x':               
                    //val64 = va_arg (ap, long long);     // val <- value to convert in ascii
                    val64_lsb = va_arg (ap, long);
                    val64_msb = va_arg (ap, long);
                    val64 = ((long long) val64_msb << 32LL) | val64_lsb;

                    tmp = arg + sizeof (arg);           // goto at the end of tmp buffer
                    *--tmp = '\0';                      // put the ending char 0
                    i = 0;                              // i is used to count the digits
                    do {
                        *--tmp = xdigit[val64 & 0xF];   // compute the unit digit
                        val64 = (unsigned long long) val64 >> 4;  // go to the next digit
                        i++;                            // digits counter
                    } while (val64);                    // until val becomes 0
                    goto copy_tmp;                      // go to copy tmp in buffer
                default:
                    *buf++ = '%';                       // invalid format 
                    if (--count == 0)
                        goto abort;
                    goto next;
                }
                goto next;*/
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

