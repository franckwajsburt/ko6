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
char Ksk_delete[] = "\b \b";
char Ksk_return[] = "\n";
char Ksk_command[] = "command for:\n\t";


void kshell (void) 
{
    char c;

    if (Ksh_start == 0) {
        tty_write (Ksh_tty, Ksh_wellcome, sizeof (Ksh_wellcome));
        tty_write (Ksh_tty, Ksh_prompt, sizeof (Ksh_prompt));
        Ksh_start = 1;
    }
    if (tty_read (0, &c, 0) == SUCCESS) {
        //kprintf ("<%d> ", c);
        switch (c) {
        case 127 :  
            if (Ksh_count) {
                Ksh_count--;
                tty_write (Ksh_tty, Ksk_delete, sizeof (Ksk_delete)); 
            }
            break;
        case '\n':
            Ksh_buffer [Ksh_count] = '\0';
            tty_write (Ksh_tty, Ksk_return, sizeof (Ksk_return)); 
            tty_write (Ksh_tty, Ksk_command, sizeof (Ksk_command)); 
            tty_write (Ksh_tty, Ksh_buffer, Ksh_count); 
            Ksh_count = '\0';
            tty_write (Ksh_tty, Ksh_prompt, sizeof (Ksh_prompt));
            break;
        default:
            if (Ksh_count < (sizeof (Ksh_buffer) - 1)) {
                Ksh_buffer [Ksh_count++] = c;
                tty_write (Ksh_tty, &c, 1); 
            }
       } 
   } 
}
