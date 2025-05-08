#include <wordlist.h>

struct wordlist *wordlist_create(void)
{
    wordlist_s *new = malloc(sizeof(*new));

    if (!new) {
        return NULL;
    }

    new->nxt = NULL;
    
    return new;
}


void wordlist_destroy(struct wordlist *wordlist)
{
    if (wordlist->nxt) wordlist_destroy(wordlist->nxt);
    free(wordlist->word);
    free(wordlist);
}


void wordlist_print(struct wordlist *wordlist)
{
    struct wordlist *p = wordlist;

    while (p) {
        PRINT("%s -> ", p->word);
        p = p->nxt;
    }

    PRINT("NULL\n");
}


struct wordlist *make_wordlist(const char * str)
{
    wordlist_s *new = wordlist_create();

    if (!new) {
        return NULL;
    }

    new->word = strdup(str);

    return new;
}


struct wordlist *wordlist_pushfront(struct wordlist * l, const char *str)
{
    wordlist_s *new = make_wordlist(str);

    if (!new) {
        return NULL;
    }

    new->nxt = l;

    return new;
}