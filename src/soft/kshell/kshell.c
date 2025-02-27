/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date        2025-02-24
  | / /(     )/ _ \     \copyright   2021 Sorbonne University
  |_\_\ x___x \___/                  https://opensource.org/licenses/MIT

  \file     kshell/kshell.c
  \author   Lili Zheng, Marco Leon, Franck Wajsburt
  \brief    kshell is ko6's command interpreter. 
            It's not a real process, since ko6 can only have one process at a time.
            However, as it is a kernel thread, it does not share its own data with the user app.
      
\*------------------------------------------------------------------------------------------------*/

#include <kernel/klibc.h>

int  Ksh_start;
int  Ksh_tty = 0;
char Ksh_buffer[256];
int  Ksh_count;

char Ksh_wellcome[] = "Wellcome to the ko6's skell\n";
char Ksh_prompt[] = "\nko6> ";
char Ksh_delete[] = "\b \b";
char Ksh_return[] = "\n";
char Ksh_command[] = "command for:\n\t";
ht_t * Ksh_ht ;


void kshell (void) 
{
    char c;
    long val;
    unsigned try;

    if (Ksh_start == 0) {
        tty_write (Ksh_tty, Ksh_wellcome, sizeof (Ksh_wellcome));
        tty_write (Ksh_tty, Ksh_prompt, sizeof (Ksh_prompt));
        Ksh_ht = ht_create (126);
        kprintf ("test %p\n", Ksh_ht);
        Ksh_start = 1;
    }
    if (tty_read (0, &c, 0) == SUCCESS) {
        switch (c) {
        case 127 :  
            if (Ksh_count) {
                Ksh_count--;
                tty_write (Ksh_tty, Ksh_delete, sizeof (Ksh_delete)); 
            }
            break;
        case '\n':
            Ksh_buffer [Ksh_count] = '\0';
            tty_write (Ksh_tty, Ksh_return, sizeof (Ksh_return)); 
            tty_write (Ksh_tty, Ksh_command, sizeof (Ksh_command)); 
            if ((val = (long)ht_get (Ksh_ht, Ksh_buffer))) {        // ht_get return NULL at first
                try = ht_set (Ksh_ht, Ksh_buffer, (void *)(++val)); // if not increment the value
            } else {
                try = ht_set (Ksh_ht, Ksh_buffer, (void *)1);       // set a new val
            }
            PANIC_IF(try == -1, "kshell hash table too small");
            Ksh_count = 0;
            kprintf ("> %s = %d\n", Ksh_buffer, val);
            tty_write (Ksh_tty, Ksh_prompt, sizeof (Ksh_prompt));
            ht_stat(Ksh_ht);
            break;
        default:
            if (isprint(c) && (Ksh_count < (sizeof (Ksh_buffer) - 1))) {
                Ksh_buffer [Ksh_count++] = c;
                tty_write (Ksh_tty, &c, 1); 
            }
       } 
   } 
}
