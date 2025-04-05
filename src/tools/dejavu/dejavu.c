/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-05
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     tools/dejavu.c
  \author   Franck Wajsburt
  \brief    count the different words from stdin to test hash table

\*------------------------------------------------------------------------------------------------*/

#include <ctype.h>
#include <htopen.h>

// call back function to print the occurence number of each words
void print_occurences (hto_t *ht, unsigned pos, void *key, void *val, void *data) 
{
    fprintf (stderr, "%u\t %-32s : %ld\n", pos, (char *)key, (long)val);    
}

int main (int argc, char * argv[])
{
    char word[32];                                              // buffer for the word
    char c = 0;                                                 // read character
    long val;
    hto_t *ht = hto_create (16,0);
    
    while (c != EOF) {                                          // while not end of stdin

        for (c=getchar();                                       // skip all non-alnum or _
            !isalnum(c) && (c!= '_') && (c!=EOF);               
            c=getchar());  

        if (c != EOF) {                                         // if there is a new word
            char *pw = word;                                    // get the new word
            *pw++ = tolower(c);                                 // the first char is already read
            for (c=getchar();isalnum(c)||(c=='_');c=getchar()){ // while c is alphanum or '_'
                if ((pw-word) < sizeof (word)-1)                // if there is enough room
                    *pw++ = tolower(c);                         // store the char
            }
            *pw = 0;                                            // end of word

            if ((val = (long)hto_get (ht, word))) {             // hto_get return NULL at first
                hto_set (ht, word, (void *)(val+1));            // if not increment the value
            } else {         
                hto_set_grow (&ht, word, (void *)1, 16);        // add a new word and grow the table
            }                                                   // if there are more than 10 tries
        }
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
