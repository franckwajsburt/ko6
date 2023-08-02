/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-01-23
  | / /(     )/ _ \     \copyright  2021-2023 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hello/main.c
  \author   Franck Wajsburt
  \brief    first program

\*------------------------------------------------------------------------------------------------*/

#include <libc.h>

int main (void)
{
    char name[64];
    fprintf( 0, "Hello world!\n");
    fprintf( 0, "What's your name? ");
    fgets( name, sizeof(name), 0);
    name[ strlen(name)-1 ]=0;
    fprintf( 0, "Hello %s!\n", name);
    return 0;
}
