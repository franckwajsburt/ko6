#include <stdio.h>
#include "kshell_yacc.h"
#include "kshell.h"

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
        printf("%s -> ", p->word);
        p = p->nxt;
    }

    printf("NULL\n");
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


struct wordlist *wordlist_pushfront(struct wordlist * l, char *str)
{
    wordlist_s *new = make_wordlist(str);

    if (!new) {
        return NULL;
    }

    new->nxt = l;

    return new;
}


stmt_s *stmt_create(void)
{
    stmt_s *new = malloc(sizeof(*new));

    if (!new) {
        return NULL;
    }

    new->nxt = NULL;
    new->t   = NULL_TYPE;

    return new;
}

void stmt_destroy(stmt_s *victim)
{
    if (!victim) return;

    switch (victim->t) {
        case EXEC_TYPE:
        case BUILT_IN_TYPE :
            wordlist_destroy(victim->stmt.simple_stmt);
            break;
        case WHILE_TYPE :
            while_stmt_destroy(victim->stmt.while_stmt);
            break;
        case IF_TYPE :
            if_stmt_destroy(victim->stmt.if_stmt);
            break;
        default: /* NULL_TYPE ? */
            break;
    }

    free(victim);
}


if_stmt_s *if_stmt_create(void)
{
    struct if_stmt *new = malloc(sizeof(*new));

    if (!new) {
        return NULL;
    }

    new->condition  = NULL;
    new->false_case = NULL;
    new->true_case  = NULL;

    return new;
}


void if_stmt_destroy(if_stmt_s *victim)
{
    if (!victim) return;

    /**
     * \todo destroy condition
     */

    if (victim->true_case) stmt_destroy(victim->true_case);

    if (victim->false_case) stmt_destroy(victim->false_case);

    free(victim);
}


while_stmt_s *while_stmt_create(void)
{
    struct while_stmt *new = malloc(sizeof(*new));

    if (!new) {
        return NULL;
    }

    new->condition = NULL;
    new->execute   = NULL;

    return new;
}

/**
 * \brief   destroy an while_stmt
 * \param   victim the while_stmt to destroy
 * \note    this fuctions destroys the victim and all the 
 *          conditional block it represents.
 *          If stmt is NULL, this function has no effect.
 *          this function may call stmt_destroy().
 *          note that the nxt attribute will remain unmodified.
 */
void while_stmt_destroy(while_stmt_s *victim)
{
    if (!victim) return;
    /**
     * if (victim->condition) 
     * \todo destroy condition
     */

    if (victim->execute) stmt_destroy(victim->execute);

    free(victim);
}


int stmt_set_simple(stmt_s *stmt, wordlist_s *w, enum stmt_type t)
{
    if (!stmt || !w) return 0;

    stmt->t = t;
    stmt->stmt.simple_stmt = w;

    return 1;
}


int stmt_set_if_stmt(stmt_s *stmt, expr_s *cond, stmt_s *t_case, stmt_s *f_case)
{
    if (!stmt || !t_case || !cond) return 0;

    if_stmt_s *new = if_stmt_create();
    new->condition = cond;
    new->true_case = t_case;
    new->false_case = f_case;
    stmt->stmt.if_stmt = new;

    return 1;
}


int stmt_set_while_stmt(stmt_s *stmt, expr_s *cond, stmt_s *t_case)
{
    if (!stmt || !cond || !t_case) return 0;

    while_stmt_s *new = while_stmt_create();
    new->condition = cond;
    new->execute = t_case;
    stmt->stmt.while_stmt = new;

    return 1;
}

/**
 * \brief   sets the next statement to be executed
 * \param   stmt a statement
 * \param   nxt the next statement
 * \return  1 on succeed. 0 on error
 * \note    
 */
int stmt_set_next(stmt_s *stmt, stmt_s *nxt)
{
    if (!stmt || !nxt) return 0;

    stmt->nxt = nxt;

    return 1;
}



int main(int argc, char **argv) {
    printf("hello, kshell! :P\n");
    yyparse();

    return 0;
}