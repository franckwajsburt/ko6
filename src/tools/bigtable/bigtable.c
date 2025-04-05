/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-05
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     bigtable.c
  \author   Franck Wajsburt
  \brief    test hash table when keys are void*

\*------------------------------------------------------------------------------------------------*/

#include <ctype.h>
#include <htopen.h>

// call back function to print the occurence number of each words
void print_occurences (hto_t *ht, unsigned pos, void * key, void *val, void *data) 
{
    fprintf (stderr, "%u\t %-7ld : %ld\n", pos, (long) key, (long)val);    
}

int main (int argc, char * argv[])
{
    if (argc != 3) {
        fprintf (stderr, "\n\tusage: %s <slots> <fill percentage>\n"
                         "\tp.ex.: \"%s 512 80\" means a 512 slots hash table, 80%% filled\n\n", 
                         argv[0], argv[0]);
        exit (1);
    }

    long val = 0;
    unsigned long nbele = atoi(argv[1]);                        // wanted slots
    unsigned long fill  = atoi(argv[2]);
    hto_t *ht = hto_create (nbele, 1);
    
    for (nbele = (nbele * fill)/100; (nbele) ; nbele--) {       // % filled
        hto_set (ht, (void *)random(), (void *)val++);          // if not increment the value
    } 

    hto_foreach (ht, print_occurences, NULL);                   // scan the table to print the words
    hto_stat (ht);                                              // then print the hash table stats
    return 0;
}
/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
