#include <ctype.h>
#include <ht_prob.h>

// call back function to print the occurence number of each words
void print_occurences (ht_t *ht, unsigned pos, const char *key, void *val, void *data) 
{
    fprintf (stderr, "%u\t %-32s : %ld\n", pos, key, (long)val);    
}

int main (int argc, char * argv[])
{
    char word[32];                                              // buffer for the word
    char c = 0;                                                 // read character
    long val;
    ht_t *ht = ht_create (16);
    
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

            if ((val = (long)ht_get (ht, word))) {              // ht_get return NULL at first
                ht_set (ht, word, (void *)(val+1));             // if not increment the value
            } else {         
                ht_set_grow (&ht, word, (void *)1, 16);         // add a new word and grow the table
            }                                                   // if there are more than 10 tries
        }
    } 

    ht_foreach (ht, print_occurences, NULL);                    // scan the table to print the words
    ht_stat (ht);                                               // then print the hash table stats
    return 0;
}
