/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-04
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     ulib/libc.c
  \author   Franck Wajsburt
  \brief    IEEE lib.c subset

\*------------------------------------------------------------------------------------------------*/

#include <libc.h>       // external function declarations

/**
 * the return address is in user space but we need to implement a better thread local storage 
 * so for now, that is the kernel that know where errno is placed
 */
int * __errno_location (void) 
{
    return (int *)syscall_fct( 0, 0, 0, 0, SYSCALL_ERRNO);
}

void perror (char *s) 
{
    if (s && *s)
        fprintf (0, "%s: %s\n", s, errno_mess[errno + 1]); // first number of errno is -1
    else
        fprintf (0, "%s\n", errno_mess[errno + 1]);
}

void exit (int status)
{
    syscall_fct( status, 0, 0, 0, SYSCALL_EXIT);        // never returns
}

int read(int fd, void *buf, int count)
{
    return syscall_fct( fd, (int)buf, count, 0, SYSCALL_READ);
}

int write(int fd, void *buf, int count)
{
    return syscall_fct( fd, (int)buf, count, 0, SYSCALL_WRITE);
}

unsigned clock (void)
{
    return syscall_fct (0, 0, 0, 0, SYSCALL_CLOCK);
}

void dcache_buf_inval (void *buf, size_t size)
{
    syscall_fct (0, 0, 0, 0, SYSCALL_DCACHEBUFINVAL);
}

void dcache_inval (void *addr)
{
    syscall_fct (0, 0, 0, 0, SYSCALL_DCACHEINVAL);
}

size_t cachelinesize (void)
{
    return syscall_fct (0, 0, 0, 0, SYSCALL_CACHELINESIZE);
}

static unsigned randnext = 1;

#if RAND_MAX > 0x7FFFFFFE
#   error k_libc.h: RAND_MAX has to be less than 0x7FFFFFFE
#endif

int rand (void)
{
    randnext = (randnext * 1103515245 + 12345) % ((unsigned) RAND_MAX + 1);
    return randnext;
}

void srand (unsigned seed)
{
    randnext = seed;
}

void *memset (void *s, int c, unsigned n)
{
    char *p = s;

    while (n--)
        *p++ = c;
    return s;
}

void *memcpy (char *dest, char *src, unsigned n)
{
  char *d = dest;
  while (n--)
    *d++ = *src++;
  return dest;
}

int strlen (char *buf)
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

void delay (unsigned nbcycles)
{
    unsigned old_time, expected_time;

    old_time = clock ();                        // get the number od cycles at first
    expected_time = old_time + nbcycles;        // compute the number of cycles to reach
    while (clock () < expected_time);           // wait for expected time
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

int fputc (int tty, int c)
{
    write (tty, &c, 1);                         // the char is in the least significant byte
    return c;
}

int fgetc (int tty)
{
    char buf[1];                                // create a buffer for the read char
    int res = read (tty, buf, 1);               // ask for one char from TTY
    if (res != 1) return 0;                     // if no char read, should return EOF
    res = write (tty, buf, 1);                  // send it back to the TTY
    if (res != 1) return 0;                     // if no char written, should return EOF
    return (int) buf[0];                        // return char read
}

int fgets (char *buf, int count, int tty)
{
    // to make the backspace, we use ANSI codes : https://www.wikiwand.com/en/ANSI_escape_code
    char *DEL = "\033[D \033[D";                // move left, then write ' ' and move left
    int res = 0;
    count--;                                    // we need to add a NUL (0) char at the end
    char c=0;

    while ((count != 0) && (c != '\n')) {       // as long as we can or need to get a char

        read (tty, &c, 1);                      // read only 1 char
        if (c == '\r')                          // if c is the carriage return (13)
            read (tty, &c, 1);                  // get the next that is line feed (10)

        if ((c == 8)||(c == 127)) {             // 8 = backspace, 127 = delete
            if (res) {                          // go back in the buffer if possible
                write (tty, DEL, 7);            // erase current char
                count++;		                // count is the remaining place
                buf--;                          // but is the next address in buffer
                res--;
            }
            continue;                           // ask for another key
        } else
            write (tty, &c, 1);                 // loop back to the tty

        *buf = c;                               // write the char into the buffer
        buf++;                                  // increments the writing pointer
        count--;                                // decrements the remaining space
        res++;                                  // increments the read char
    }
    *buf = 0;                                   // add a last 0 to end the string

    return res;                                 // returns the number of char read
}

static int vsnprintf (char * buffer, unsigned size, char *fmt, va_list ap)
{
    char arg[16];                               // buffer used to build the argument
    char *tmp;                                  // temporary pointer used to build arguments
    char *buf = buffer;                         // pointer to the current filling position in buffer
    int res;                                    // function result (number of printed char)
    int val;                                    // argument value
    int i;                                      // temporary variable
    int count = size - 1;                       // max nb of char in buffer (-1 because of last 0)
    char xdigit[] = "0123456789abcdef";         // hexadecimal digit

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

int fprintf(int tty, char *fmt, ...)
{
    static char buffer[PRINTF_MAX];
    int res;
    va_list ap;
    va_start (ap, fmt);
    res = vsnprintf(buffer, sizeof(buffer), fmt, ap);
    res = write( tty, buffer, res);
    va_end(ap);
    return res;
}

int snprintf(char *str, unsigned size, char *fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
    int res = vsnprintf(str, size, fmt, ap);
    va_end(ap);
    return res;
}
