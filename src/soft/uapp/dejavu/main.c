#include <libc.h>

int stderr = 1;             // FIXME it should be a file descriptor, not a tty number
int stdin = 1;              // FIXME it should be a file descriptor, not a tty number
#define EOF '.'             // FIXME should be -1 after a ctrl-D
#define getc fgetc(stdin)   

// call back function to print the occurence number of each words
void print_occurences (ht_t *ht, size_t pos, const char *key, void *val, void *data) 
{
    fprintf (stderr, "%u\t %s : %d\n", pos, key, (long)val);    
}


int main (int argc, char * argv[])
{
    char word[32];                                              // buffer for the word
    char c = EOF;                                                 // read character
    long val;
    ht_t *ht = ht_create (16);
    
    while (c != EOF) {                                          // while not end of stdin

        for (c=getc;                                            // skip all non-alnum or _
            !isalnum(c) && (c!= '_') && (c!=EOF);               
            c=getc);  

        if (c != EOF) {                                         // if there is a new word
            char *pw = word;                                    // get the new word
            *pw++ = tolower(c);                                 // the first char is already read
            for (c=getc; isalnum(c) || (c=='_'); c=getc) {      // while c is alphanum or '_'
                if ((pw-word) < sizeof (word)-1)                // if there is enough room
                    *pw++ = tolower(c);                         // store the char
            }
            *pw = 0;                                            // end of word

            if ((val = (long)ht_get (ht, word))) {              // ht_get return NULL at first
                ht_set (ht, word, (void *)(val+1));             // if not increment the value
            } else {         
                ht_set_grow (&ht, word, (void *)1, 10);         // add a new word and grow the table
            }                                                   // if there are more than 10 tries
        }
    } 

    ht_foreach (ht, print_occurences, NULL);                    // scan the table to print the words
    ht_stat (ht);                                               // then print the hash table stats
    return 0;
}
