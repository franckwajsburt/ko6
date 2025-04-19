
/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-18
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     uapp/kshell/kshell.c
  \author   Lili Zheng, Marco Leon, Franck Wajsburt
  \brief    

\*------------------------------------------------------------------------------------------------*/

#include <libc.h>

int split(char *str, char *tokens[], int maxtoken) {
    int count = 0;
    while (*str == ' ' || *str == '\t') str++;
    while (*str != '\0' && count < maxtoken) {
        tokens[count++] = str;
        while (*str != '\0' && *str != ' ' && *str != '\t') str++;
        if (*str != '\0') {
            *str++ = '\0';
            while (*str == ' ' || *str == '\t') str++;
        }
    }
    return count;
}

void exec (char * buffer)
{
    char *tokens[16];
    kshell_args_t args;
    int count = split (buffer, tokens, sizeof (tokens)/sizeof(char*));

    if (strcmp (tokens[0], "open") == 0) {
        if (count != 3) {
            fprintf (1, "usage: open pathname flags\n");
            return;
        }
        args.a_open.path = tokens[1];
        args.a_open.flags = atoi(tokens[2]);
        syscall_fct (KSHELL_OPEN, (unsigned)&args, 0, 0, SYSCALL_KSHELL);
        fprintf (1, "open result: %d\n", args.a_open.resfd);
    }
}

void kcmd (int tty) 
{
    static int  start;

    static char prompt[] = "\nko6> ";
    static char delete[] = "\b \b";
    static char cr[] = "\n";
    static char buffer[256];
    static int  count;
    static hto_t * hto ;

    char c;
    long val;
    unsigned try;
    if (start == 0) { //------------------- Only once
        write (tty, prompt, sizeof (prompt));
        hto = hto_create (128, 0);
        start = 1;
    }
    if (read (tty, &c, 0) == SUCCESS) { // ask 0 char means non-blocking behavior
        switch (c) {

        case 127 :  // -------------------- back space
            if (count) {
                count--;
                write (tty, delete, sizeof (delete)); 
            }
            break;

        case '\n': // --------------------- carriage return
            buffer [count] = '\0';
            write (tty, cr, sizeof (cr)); 
            if ((val = (long)hto_get (hto, buffer))) {        // hto_get return NULL at first
                try = hto_set (hto, buffer, (void *)(++val)); // if not increment the value
            } else {
                try = hto_set (hto, buffer, (void *)1); // set a new val
                val = 1;
            }
            exec (buffer);

            PANIC_IF (try == -1, "kshell hash table too small");
            count = 0;
            fprintf (tty, "%s = %d\n", buffer, val);
            write (tty, prompt, sizeof (prompt));
            //hto_stat (hto);
            break;

        default: // ----------------------- register char and loopback
            if (isprint (c) && (count < (sizeof (buffer) - 1))) {
                buffer [count++] = c;
                write (tty, &c, 1); 
            }
        } 
   } 
}

int main () 
{
    fprintf (1, "hello, friend\n");
    while (1) kcmd (1);
    syscall_fct (KSHELL_OPEN, (unsigned) NULL, 0, 0, SYSCALL_KSHELL);

    return 0;
}

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
