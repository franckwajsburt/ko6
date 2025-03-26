#include <stdio.h>
#include "kshell_yacc.h"
#include "kshell.h"

struct wordlist *wordlist_create(void)
{
    wordlist_s *new = malloc(sizeof(*new));
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
        printf("%s -> ", p->word);
        p = p->nxt;
    }

    printf("NULL\n");
}


struct wordlist *make_wordlist(const char * str)
{
    wordlist_s *new = wordlist_create();
    new->word = strdup(str);

    return new;
}


struct wordlist *wordlist_pushfront(struct wordlist * l, char *str)
{
    wordlist_s *new = make_wordlist(str);
    new->nxt = l;

    return new;
}



int main(int argc, char **argv) {
    printf("hello, kshell! :P\n");
    yyparse();

    return 0;
}