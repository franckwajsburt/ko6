#include <stmt.h>

stmt_s *stmt_create(void)
{
    stmt_s *new = MALLOC(sizeof(*new));

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
    struct if_stmt *new = MALLOC(sizeof(*new));

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
    struct while_stmt *new = MALLOC(sizeof(*new));

    if (!new) {
        return NULL;
    }

    new->condition = NULL;
    new->execute   = NULL;

    return new;
}


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
            PRINT("built-in: ");
            wordlist_print(stmt->stmt.simple_stmt);
            break;
        case WHILE_TYPE:
            PRINT("while\n");
            break;
        case EXPR_TYPE:
            PRINT("expr: ");
            expr_print(stmt->stmt.expr);
            break;
        case IF_TYPE:
            PRINT("if\ncond:\n");
            stmt_print(stmt->stmt.if_stmt->condition);
            PRINT("true: ");
            stmt_print(stmt->stmt.if_stmt->branch[1]);
            if (stmt->stmt.if_stmt->branch[0]) {
                PRINT("else: ");
                stmt_print(stmt->stmt.if_stmt->branch[0]);
            }
            break;
        case PIPELINE_TYPE:
            PRINT("\t -> pipeline\n");
            break;
        case EXEC_TYPE:
            PRINT("exec: ");
            wordlist_print(stmt->stmt.simple_stmt);
            break;
        case ENV_ASSIGN_TYPE:
            PRINT("env assignmt: ");
            wordlist_print(stmt->stmt.simple_stmt);
            break;
        default:
            PRINT("fck you\n");
    }

    if (stmt->nxt) {
        PRINT("nxt: ");
        stmt_print(stmt->nxt);
    }
    PRINT("end!\n");
}

struct expr *expr_create(void)
{
    expr_s *new = MALLOC(sizeof(*new));

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

int expr_set_word(expr_s *expr, const char *w)
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
    struct q_t *q = MALLOC(sizeof(*q));

    if (!q) exit(1);

    q->b = MALLOC(sz * sizeof(*q->b));

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
            PRINT("NULL\n");
            break;
        case AND_OP:
            PRINT("AND\n");
            break;
        case OR_OP:
            PRINT("OR\n");
            break;
        case PLUS_OP:
            PRINT("PLUS\n");
            break;
        case MINUS_OP:
            PRINT("MINUS\n");
            break;
        case MULT_OP:
            PRINT("MULT\n");
            break;
        case DIV_OP:
            PRINT("DIV\n");
            break;
        case EQ_OP:
            PRINT("EQ\n");
            break;
        case NEQ_OP:
            PRINT("NEQ\n");
            break;
        case ASSIGN_OP:
            PRINT("ASSIGN\n");
            break;
        case LT_OP:
            PRINT("LT\n");
            break;
        case GT_OP:
            PRINT("GT\n");
            break;
        case LEQ_OP:
            PRINT("LEQ\n");
            break;
        case GEQ_OP:
            PRINT("GEQ\n");
            break;
        case NOT_OP:
            PRINT("NOT\n");
            break;
        case INT_EXPR:
            PRINT("%d\n", expr->v.v);
            return;
        case WORD_EXPR:
            PRINT("%s\n", expr->v.word);
            return;
        case STMT_EXPR:
            stmt_print(expr->v.stmt);
            return;
        default:
            break;
        }

        for (int i = 0; i < q->h; i++)
            if (q->b[i]) PRINT(" \u2502");
            else PRINT("   ");
        
        PRINT(" \u251c");
        q_push(q, 1);
        expr_print_(expr->v.e[0], q);
        q_pop(q);

        for (int i = 0; i < q->h; i++)
            if (q->b[i]) PRINT(" \u2502");
            else PRINT("   ");
        
        PRINT(" \u2514");
        q_push(q, 0);
        expr_print_(expr->v.e[1], q);
        q_pop(q);
    } else {
        PRINT("\u259e\n");
    }
}

void expr_print(expr_s *expr)
{
    if (!expr) PRINT("\u259e");
    
    struct q_t *q = q_create(64);
    expr_print_(expr, q);
    q_destroy(q);
}

char *expr_eval(expr_s *expr)
{
    return NULL;
}


long expr_eval_long(expr_s *expr)
{
    return 0;
}