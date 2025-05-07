/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     uapp/kshell/kshell.c
  \author   Lili Zheng, Marco Leon, Franck Wajsburt
  \brief    ko6 shell

\*------------------------------------------------------------------------------------------------*/

#include <kshell.h>
#include "kshell_yacc.h"
#include <varenv.h>
#include <wordlist.h>
#include <stmt.h>

/* execution */

stmt_s *curr;
stmt_s *chkpnt;
expr_s *aux;
hto_t *envars; 			/* environment variables here */


int kshell_expr_eval(expr_s *expr)
{
    if (!expr) {
        /* FIXME: HANDLE THIS PROPERLY */
        PRINT("expr is NULL. returning 0\n");
        return 0;
    }

    int retval = 0;
    varenv_s *v;

    switch (expr->t)
    {
    case AND_OP:
        retval = kshell_expr_eval(expr->v.e[0]);

        if (retval) {
            retval = retval && kshell_expr_eval(expr->v.e[1]);
        }

        break;
    case OR_OP:
        retval = kshell_expr_eval(expr->v.e[0]);

        if (!retval) {
            retval = retval || kshell_expr_eval(expr->v.e[1]);
        }

        break;
    case PLUS_OP:
        retval = kshell_expr_eval(expr->v.e[0]) + kshell_expr_eval(expr->v.e[0]);
        break;
    case MINUS_OP:
        retval = kshell_expr_eval(expr->v.e[0]) - kshell_expr_eval(expr->v.e[1]);
        break;
    case MULT_OP:
        retval = kshell_expr_eval(expr->v.e[0]) * kshell_expr_eval(expr->v.e[1]);
        break;
    case DIV_OP:
        retval = kshell_expr_eval(expr->v.e[1]);
        
        if (retval != 0) {
            retval = kshell_expr_eval(expr->v.e[1]) / retval;
        } else {
            retval = 0; 
        }

        break;
    case EQ_OP:
        retval = kshell_expr_eval(expr->v.e[0]) == kshell_expr_eval(expr->v.e[1]);
        break;
    case NEQ_OP:
        retval = kshell_expr_eval(expr->v.e[0]) != kshell_expr_eval(expr->v.e[1]);
        break;
    case ASSIGN_OP:
        /* TODO: VERIFY THIS  */
        retval = 0;
        break;
    case LT_OP:
        retval = kshell_expr_eval(expr->v.e[0]) < kshell_expr_eval(expr->v.e[1]);
        break;
    case GT_OP:
        retval = kshell_expr_eval(expr->v.e[0]) > kshell_expr_eval(expr->v.e[1]);
        break;
    case LEQ_OP:
        retval = kshell_expr_eval(expr->v.e[0]) <= kshell_expr_eval(expr->v.e[1]);
        break;
    case GEQ_OP:
        retval = kshell_expr_eval(expr->v.e[0]) >= kshell_expr_eval(expr->v.e[1]);
        break;
    case NOT_OP:
        retval = kshell_expr_eval(expr->v.e[0]);
        break;
    case INT_EXPR:
        retval = expr->v.v;
        break;
    case WORD_EXPR:
        /* every word will have value 0 */
        
        v = hto_get(envars, expr->v.word);
        
        if (!v) break;

        retval = atoi(v->v); 
        break;
    case STMT_EXPR:
        /*
        1. execute statement
        2. print whatever the fuck you need to print
        3. return the output state of the execution
           dont forget to convert the result to kshell bool val
        */
        retval = 0;
        break;
    default:
        PRINT("fuck you!\n");
        /* FIXME:  maybe you can handle this better */
        break;
    }

    return retval;
}

int kshell_stmt_execute(stmt_s *stmt)
{
    if (!stmt) {
        PRINT("stmt is NULL. Returning 0");
        return 0;
    }

    int retval = 0;
    wordlist_s *w;
    stmt_s *curr = stmt;

    while (curr) {
        switch(curr->t)
        {
        case WHILE_TYPE:
            retval = kshell_while_stmt_execute(curr->stmt.while_stmt);
            break;
        case IF_TYPE:
            retval = kshell_if_stmt_execute(curr->stmt.if_stmt);
            break;
        case PIPELINE_TYPE:
            PRINT("pipeline: NOT IMPLEMENTED YET\n");
            break;
        case BUILT_IN_TYPE:
            retval = kshell_built_in_execute(curr);
            break;
        case EXEC_TYPE:
            PRINT("exec: NOT IMPLEMENTED YET\n");
            break;
        case EXPR_TYPE:
            retval = kshell_expr_eval(curr->stmt.expr);
            break;
        case ENV_ASSIGN_TYPE:
            w = curr->stmt.simple_stmt;
            kshell_set_varenv(w->word, w->nxt->word, 0);
            break;
        default:
            break;
        }
        curr = curr->nxt;
    }

    return retval;
}

int kshell_while_stmt_execute(while_stmt_s *wstmt)
{
    int retval = 0;

    while (kshell_stmt_execute(wstmt->condition)) {
        retval = kshell_stmt_execute(wstmt->execute);
    }

    return retval;
}

int kshell_if_stmt_execute(if_stmt_s *istmt)
{
    int b = kshell_stmt_execute(istmt->condition);
    PRINT("b: %d\n", b);

    return kshell_stmt_execute(istmt->branch[!!b]);
}

int kshell_built_in_execute(stmt_s *bstmt)
{
    /* not so elegant, but it should work */
    if (0 == strcmp(bstmt->stmt.simple_stmt->word, "ls")) {
        return kshell_ls(bstmt->stmt.simple_stmt->nxt);
    }

    if (0 == strcmp(bstmt->stmt.simple_stmt->word, "cat")) {
        return kshell_cat(bstmt->stmt.simple_stmt->nxt);
    }

    if (0 == strcmp(bstmt->stmt.simple_stmt->word, "export")) {
        return kshell_export(bstmt->stmt.simple_stmt->nxt);
    }

    if (0 == strcmp(bstmt->stmt.simple_stmt->word, "pwd")) {
        return kshell_pwd(bstmt->stmt.simple_stmt->nxt);
    }

    if (0 == strcmp(bstmt->stmt.simple_stmt->word, "cd")) {
        return kshell_cd(bstmt->stmt.simple_stmt->nxt);
    }

    if (0 == strcmp(bstmt->stmt.simple_stmt->word, "echo")) {
        return kshell_echo(bstmt->stmt.simple_stmt->nxt);
    }

    if (0 == strcmp(bstmt->stmt.simple_stmt->word, "kill")) {
        return kshell_kill(bstmt->stmt.simple_stmt->nxt);
    }

    if (0 == strcmp(bstmt->stmt.simple_stmt->word, "su")) {
        return kshell_su(bstmt->stmt.simple_stmt->nxt);
    }

    if (0 == strcmp(bstmt->stmt.simple_stmt->word, "top")) {
        return kshell_top(bstmt->stmt.simple_stmt->nxt);
    }

    if (0 == strcmp(bstmt->stmt.simple_stmt->word, "kshell")) {
        return kshell_kshell(bstmt->stmt.simple_stmt->nxt);
    }

    if (0 == strcmp(bstmt->stmt.simple_stmt->word, "kvar")) {
        return kshell_kvar(bstmt->stmt.simple_stmt->nxt);
    }

    return 0;
}

int kshell_ls(wordlist_s *args)
{
    PRINT("ls: NOT IMPLEMENTED BUILT-IN. args: ");
    wordlist_print(args);

    return 1;
}
int kshell_cat(wordlist_s *args)
{
    PRINT("cat: NOT IMPLEMENTED BUILT-IN. args: ");
    wordlist_print(args);

    return 1;
}
int kshell_echo(wordlist_s *args)
{
    PRINT("echo: NOT IMPLEMENTED BUILT-IN. args: ");
    wordlist_print(args);

    return 1;
}
int kshell_export(wordlist_s *args)
{
    PRINT("export NOT IMPLEMENTED BUILT-IN. args: ");
    wordlist_print(args);

    return 1;
}
int kshell_kshell(wordlist_s *args)
{
    PRINT("kshell: NOT IMPLEMENTED BUILT-IN. args: ");
    wordlist_print(args);

    return 1;
}
int kshell_cd(wordlist_s *args)
{
    PRINT("cd: NOT IMPLEMENTED BUILT-IN. args: ");
    wordlist_print(args);

    return 1;
}
int kshell_pwd(wordlist_s *args)
{
    PRINT("pwd: NOT IMPLEMENTED BUILT-IN. args: ");
    wordlist_print(args);

    return 1;
}
int kshell_top(wordlist_s *args)
{
    PRINT("top: NOT IMPLEMENTED BUILT-IN. args: ");
    wordlist_print(args);

    return 1;
}
int kshell_kill(wordlist_s *args)
{
    PRINT("kill: NOT IMPLEMENTED BUILT-IN. args: ");
    wordlist_print(args);

    return 1;
}
int kshell_kvar(wordlist_s *args)
{
    PRINT("kvar: NOT IMPLEMENTED BUILT-IN. args: ");
    wordlist_print(args);

    return 1;
}
int kshell_su(wordlist_s *args)
{
    PRINT("su: NOT IMPLEMENTED BUILT-IN. args: ");
    wordlist_print(args);

    return 1;
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



/* the next code is just for the kshell */


int kshell_set_varenv(char *n, const char *v, int flags)
{
    if (!envars) return 0;

    struct varenv *var = varenv_create();
    varenv_value_set(var, v);
    varenv_attr_set(var, flags);

    return hto_set(envars, n, var);
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

void print_helper(hto_t *ht, unsigned pos, void *key, void *val, void *data)
{
    PRINT("%d\t %s : %s\n", pos, (char *)key, ((varenv_s *)val)->v);
}

void kshell_print_env()
{
    hto_foreach(envars, print_helper, NULL);

}

hto_t *envars;


int main(int argc, char **argv) {
    envars = hto_create(71, 0);
    PRINT("hello, kshell! :)\n");

    if (!yyparse()) {
        PRINT("parsed!\n");
    } else {
        PRINT("wut ?\n");
    }

    return 0;
}
/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
