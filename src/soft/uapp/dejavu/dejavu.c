#include <libc.h>

int stdout = 1;             // FIXME it should be a file descriptor, not a tty number
int stdin = 1;              // FIXME it should be a file descriptor, not a tty number
#define EOT 4               // FIXME should be -1 after a ctrl-D
#define getc fgetc(stdin)   

// call back function to print the occurence number of each words
void print_occurences (ht_t *ht, size_t pos, void *key, void *val, void *data) 
{
    fprintf (stdout, "%d\t %s : %d\n", pos, (char *)key, (long)val);    
}

int main (int argc, char * argv[])
{
    fprintf (stdout, "Type any words ended with <CTRL-D>\n");    

    char word[32];                                              // buffer for the word
    char c = getc;                                              // read character
    long val;
    ht_t *ht = ht_create (16, 0);

    while (c != EOT) {                                          // while not end of stdin

        for (;!isalnum(c) && (c!= '_') && (c!=EOT); c=getc);    // skip all non-alnum or _      

        if (c != EOT) {                                         // if there is a new word
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

    fprintf (stdout,"\n");
    ht_foreach (ht, print_occurences, NULL);                    // scan the table to print the words
    ht_stat (ht);                                               // then print the hash table stats
    return 0;
}
