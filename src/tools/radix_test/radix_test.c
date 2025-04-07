/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-06
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     tools/radix_test.c
  \author   Franck Wajsburt
  \brief    test radix tree

\*------------------------------------------------------------------------------------------------*/

#include <ctype.h>
#include <radix.h>

#define V(v) fprintf( stderr, "%s : %lu\n", #v, (unsigned long)v);

// call back function to print the occurence number of each words
void print_occurences (const radix_t *rx, unsigned index, void *val, void *data) 
{
    fprintf (stderr, "%-7ld : %ld\n", (long) index, (long)val);    
}

int main (int argc, char * argv[])
{
    if (argc != 2) {
        fprintf (stderr, "\n\tusage: %s <values>\n"
                         "\tp.ex.: \"%s 512\" means a 512 values filled\n\n", 
                         argv[0], argv[0]);
        exit (1);
    }

    unsigned long val = 0;
    unsigned long nbele = atoi(argv[1]);
    radix_t *rx = radix_create ();
    
    for (int i = nbele; i>0 ; i--) {
        radix_set (rx, i*10, (void *)val++);
        val++;
    } 

    radix_set (rx,       0x30, (void *)0xDEAD3); 
    radix_set (rx,       0x31, (void *)0xDEAD3);
    radix_set (rx,     0x2000, (void *)0xDEAD2);
    radix_foreach (rx,  print_occurences, NULL);
    radix_export_dot (rx, "test.dot");
    radix_stat (rx);
    radix_set (rx,   0x1000000, (void *)0xDEAD1);
    radix_set (rx,   0x2000000, (void *)0xDEAD2);
    radix_set (rx,   0x2000000, NULL);
    radix_cleanup(rx);
    radix_foreach (rx,  print_occurences, NULL);
    radix_export_dot (rx, "test2.dot");
    radix_stat (rx);
    radix_set (rx, 0x10000000, (void *)0xDEAD0);
    radix_foreach (rx,  print_occurences, NULL);

    for (int i = nbele; i>0 ; i-=2) {
        radix_set (rx, i*10, NULL);
        val++;
    } 

    return 0;
}
/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
