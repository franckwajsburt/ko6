#include <ctype.h>
#include <ht_prob.h>

// call back function to print the occurence number of each words
void print_occurences (ht_t *ht, unsigned pos, void * key, void *val, void *data) 
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
    ht_t *ht = ht_create (nbele, 1);
    
    for (nbele = (nbele * fill)/100; (nbele) ; nbele--) {   // % filled
        ht_set (ht, (void *)random(), (void *)val++);           // if not increment the value
    } 

    ht_foreach (ht, print_occurences, NULL);                    // scan the table to print the words
    ht_stat (ht);                                               // then print the hash table stats
    return 0;
}
