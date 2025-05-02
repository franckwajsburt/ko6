/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date        2025-03-30
  | / /(     )/ _ \     \copyright   2021 Sorbonne University
  |_\_\ x___x \___/                  https://opensource.org/licenses/MIT

  \file     /tools/kshell/kshell.c
  \author   Marco Leon
  \brief    a shell interpreter for ko6 based on F. Normand's previous work

  State :   building
  
  NB :      :P


\*------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "kshell_yacc.h"
#include "kshell.h"

struct expr *expr_create(void)
{
    expr_s *new = malloc(sizeof(*new));

    if (!new) {
        return NULL;
    }

    new->t = NULL_EXPR;

    return new;
}


void expr_destroy(expr_s *victim)
{
    if (!victim) return;

    switch (victim->t)
    {
    case AND_OP :
    case OR_OP :
    case PLUS_OP :
    case MINUS_OP :
    case MULT_OP :
    case DIV_OP :
    case LT_OP :
    case GT_OP :
    case LEQ_OP :
    case GEQ_OP :
    case EQ_OP :
    case NEQ_OP :
    case ASSIGN_OP :
        expr_destroy(victim->v.e[0]);
        expr_destroy(victim->v.e[1]);
        break;
    case NOT_OP :
        expr_destroy(victim->v.e[0]);
        break;
    case INT_EXPR :
        break;
    case WORD_EXPR :
        free(victim->v.word);
        break;
    case STMT_EXPR :
        stmt_destroy(victim->v.stmt);
        break;
    case NULL_EXPR:
    default:
        break;
    }

    free(victim);
}


int expr_set_op(expr_s *expr, expr_type_e op, expr_s *l, expr_s *r)
{
    if (!expr || !l) return 0;

    switch (op)
    {
    case AND_OP :
    case OR_OP :
    case PLUS_OP :
    case MINUS_OP :
    case MULT_OP :
    case DIV_OP :
    case LT_OP :
    case GT_OP :
    case LEQ_OP :
    case GEQ_OP :
    case EQ_OP :
    case NEQ_OP :
    case ASSIGN_OP :
        if (!r) return 0;
        expr->v.e[0] = l;
        expr->v.e[1] = r;
        break;
    case NOT_OP :
        expr->v.e[0] = l;
        break;
    default:
        break;
    }

    expr->t = op;

    return 1;
}

int expr_set_var(expr_s *expr, const char *w)
{
    if (!expr || !w) return 0;

    expr->t = WORD_EXPR;
    expr->v.word = strdup(w);

    return 1;
}

int expr_set_int(expr_s *expr, int v)
{
    if (!expr) return 0;

    expr->t = INT_EXPR;
    expr->v.v = v;

    return 1;
}


int expr_set_stmt(expr_s *expr, stmt_s *stmt)
{
    if (!expr || !stmt) return 0;

    expr->t = STMT_EXPR;
    expr->v.stmt = stmt;

    return 1;
}

struct q_t {
    int   *b;   /*   bits */
    size_t h;   /* height */
};

struct q_t *q_create(size_t sz)
{
    struct q_t *q = malloc(sizeof(*q));

    if (!q) exit(1);

    q->b = malloc(sz * sizeof(*q->b));

    if (!q->b) exit(1);

    q->h = 0;

    return q;
}

void q_destroy(struct q_t *q)
{
    if (q->b) free(q->b);

    free(q);
}

void q_push(struct q_t *q, int bit)
{
    q->b[q->h++] = bit;
}

int q_pop(struct q_t *q)
{
    if (q->h > 0) return q->b[--q->h];
    else return -1;
}

void expr_print_(expr_s *expr, struct q_t *q)
{
    if (expr) {
        switch (expr->t)
        {
        case NULL_EXPR:
            printf("NULL\n");
            break;
        case AND_OP:
            printf("AND\n");
            break;
        case OR_OP:
            printf("OR\n");
            break;
        case PLUS_OP:
            printf("PLUS\n");
            break;
        case MINUS_OP:
            printf("MINUS\n");
            break;
        case MULT_OP:
            printf("MULT\n");
            break;
        case DIV_OP:
            printf("DIV\n");
            break;
        case EQ_OP:
            printf("EQ\n");
            break;
        case NEQ_OP:
            printf("NEQ\n");
            break;
        case ASSIGN_OP:
            printf("ASSIGN\n");
            break;
        case LT_OP:
            printf("LT\n");
            break;
        case GT_OP:
            printf("GT\n");
            break;
        case LEQ_OP:
            printf("LEQ\n");
            break;
        case GEQ_OP:
            printf("GEQ\n");
            break;
        case NOT_OP:
            printf("NOT\n");
            break;
        case INT_EXPR:
            printf("%d\n", expr->v.v);
            return;
        case WORD_EXPR:
            printf("%s\n", expr->v.word);
            return;
        case STMT_EXPR:
            stmt_print(expr->v.stmt);
            return;
        default:
            break;
        }

        for (int i = 0; i < q->h; i++)
            if (q->b[i]) printf(" \u2502");
            else printf("   ");
        
        printf(" \u251c");
        q_push(q, 1);
        expr_print_(expr->v.e[0], q);
        q_pop(q);

        for (int i = 0; i < q->h; i++)
            if (q->b[i]) printf(" \u2502");
            else printf("   ");
        
        printf(" \u2514");
        q_push(q, 0);
        expr_print_(expr->v.e[1], q);
        q_pop(q);
    } else {
        printf("\u259e\n");
    }
}

void expr_print(expr_s *expr)
{
    if (!expr) printf("\u259e");
    
    struct q_t *q = q_create(64);
    expr_print_(expr, q);
    q_destroy(q);
}

char *eval_expr(expr_s *expr)
{
    return NULL;
}


int eval_expr_int(expr_s *expr)
{
    return 0;
}

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


struct wordlist *wordlist_pushfront(struct wordlist * l, const char *str)
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
        case ENV_ASSIGN_TYPE :
            wordlist_destroy(victim->stmt.simple_stmt);
            break;
        case WHILE_TYPE :
            while_stmt_destroy(victim->stmt.while_stmt);
            break;
        case IF_TYPE :
            if_stmt_destroy(victim->stmt.if_stmt);
            break;
        case EXPR_TYPE:
            expr_destroy(victim->stmt.expr);
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

    new->condition = NULL;
    new->branch[0] = NULL;
    new->branch[1] = NULL;

    return new;
}


void if_stmt_destroy(if_stmt_s *victim)
{
    if (!victim) return;

    /**
     * \todo destroy condition
     */

    if (victim->branch[0]) stmt_destroy(victim->branch[0]);

    if (victim->branch[1]) stmt_destroy(victim->branch[1]);

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


int stmt_set_expr(stmt_s *stmt, expr_s *expr)
{
    if (!stmt || !expr) return 0;

    stmt->t = EXPR_TYPE;
    stmt->stmt.expr = expr;

    return 1;
}

int stmt_set_simple(stmt_s *stmt, wordlist_s *w, enum stmt_type t)
{
    if (!stmt || !w) return 0;

    stmt->t = t;
    stmt->stmt.simple_stmt = w;

    return 1;
}


int stmt_set_if_stmt(stmt_s *stmt, stmt_s *cond, stmt_s *t_case, stmt_s *f_case)
{
    if (!stmt || !t_case || !cond) return 0;

    if_stmt_s *new = if_stmt_create();
    new->condition = cond;
    new->branch[1] = t_case;
    new->branch[0] = f_case;
    stmt->stmt.if_stmt = new;
    stmt->t = IF_TYPE;

    return 1;
}


int stmt_set_while_stmt(stmt_s *stmt, stmt_s *cond, stmt_s *t_case)
{
    if (!stmt || !cond || !t_case) return 0;

    while_stmt_s *new = while_stmt_create();
    new->condition = cond;
    new->execute = t_case;
    stmt->stmt.while_stmt = new;
    stmt->t = WHILE_TYPE;

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

void stmt_print(stmt_s *stmt)
{
    if (!stmt) return;

    switch (stmt->t) {
        case BUILT_IN_TYPE:
            printf("built-in: ");
            wordlist_print(stmt->stmt.simple_stmt);
            break;
        case WHILE_TYPE:
            printf("while\n");
            break;
        case EXPR_TYPE:
            printf("expr: ");
            expr_print(stmt->stmt.expr);
            break;
        case IF_TYPE:
            printf("if\ncond:\n");
            stmt_print(stmt->stmt.if_stmt->condition);
            printf("true: ");
            stmt_print(stmt->stmt.if_stmt->branch[1]);
            if (stmt->stmt.if_stmt->branch[0]) {
                printf("else: ");
                stmt_print(stmt->stmt.if_stmt->branch[0]);
            }
            break;
        case PIPELINE_TYPE:
            printf("\t -> pipeline\n");
            break;
        case EXEC_TYPE:
            printf("exec: ");
            wordlist_print(stmt->stmt.simple_stmt);
            break;
        case ENV_ASSIGN_TYPE:
            printf("env assignmt: ");
            wordlist_print(stmt->stmt.simple_stmt);
            break;
        default:
            printf("fck you\n");
    }

    if (stmt->nxt) {
        printf("nxt: ");
        stmt_print(stmt->nxt);
    }
    printf("end!\n");
}


int i2kbool(int v)
{
    if (v) return 0;
    else return 1;
}


int kbool2i(int v)
{
    return !v;
}


struct varenv *varenv_create(void)
{
    struct varenv *new = malloc(sizeof*new);
    new->v = NULL;
    new->attr = 0;

    return new;
}


void varenv_destroy(struct varenv *victim)
{
    if (!victim) return;

    if (victim->v) free(victim->v);

    free(victim);
}


int varenv_value_set(struct varenv *var, const char * v)
{
    if (!var) return 0;

    if (var->v) free(var->v);

    var->v = strdup(v);
    var->attr = 0;

    return 1;
}


int varenv_attr_set(struct varenv *var, int mask)
{
    if (!var) return 0;

    var->attr |= mask; // note that it's just a logic OR

    return 1;
}


int varenv_attr_unset(struct varenv *var, int mask)
{
    if (!var) return 0;

    var->attr &= ~mask;

    return 1;
}

/* the next code is just for the kshell */

int kshell_record_varenv(ht, n, v, flags)
hto_t *ht;
char *n;
const char *v;
int flags;
{
    if (!ht) return 0;

    struct varenv *var = varenv_create();
    varenv_value_set(var, v);
    varenv_attr_set(var, flags);

    return hto_set(ht, n, var);
}

int kshell_unset_varenv(ht, name)
hto_t *ht;
char *name;
{
    if (!ht) return -1;

    varenv_s *victim = hto_del(ht, name);
    varenv_destroy(victim);

    return 0;
}

void varenv_print(hto_t *ht, unsigned pos, void *key, void *val, void *data)
{
    printf("%d\t %s : %s\n", pos, (char *)key, (char *)val);    
}

void kshell_print_env(hto_t *ht)
{
    hto_foreach(ht, varenv_print, NULL);

}

hto_t *envars;


int main(int argc, char **argv) {
    envars = hto_create(71, 0);
    printf("hello, kshell! :)\n");

    if (!yyparse()) {
        printf("parsed!\n");
    } else {
        printf("wut ?\n");
    }

    return 0;
}