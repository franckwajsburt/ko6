/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     /tools/kshell/kshell.h
  \author   Marco Leon
  \brief    here you'll find the headers and fux declarations regarding the 
            kshell. You may see some notes, you can ignore them, they are 
            probably lame ideas.

  State :   building
  
  NB :      :P

\*------------------------------------------------------------------------------------------------*/

#ifndef _KSHELL_H_
#define _KSHELL_H_

/**
 * You're gonna need a returning value
 * convention
 * 
 * 0 -> ok yippee
 * other -> no ok no yippee
 * 
 * those structures are shamelessly plumdered from 
 * the Francois Normand's code that implemented a
 * ast in order to parse the instructions. Even 
 * if the control flow was not implemented, the 
 * basic idea remains. So, if you feel stuck, take 
 * a look to Normand's code.
 */

 #ifdef _HOST_
 #   include <stdio.h>
 #   include <stdlib.h>
 #   include <stdint.h>
 #   include <fcntl.h>
 #   include <unistd.h>
 #   include <string.h>
 #   include <sys/types.h>
 #   include <stdint.h>
 #   define MALLOC(s) malloc(s)
 #   define FREE(s) free(s)
 #   define P(fmt,var) fprintf(stderr, #var" : "fmt, var)
 #   define RETURN(e,c,m,...) if(c){fprintf(stderr,"Error "m"\n");__VA_ARGS__;return e;}
 #   define OPENR(f) open (f, O_RDONLY)
 #   define OPENW(f) open (f, O_WRONLY | O_CREAT | O_TRUNC, 0644)
 #   define PRINT(fmt,...) printf(fmt,__VA_ARGS__)
 #else
 #   define MALLOC(s) kmalloc(s)
 #   define FREE(s) kfree(s)
 #   define P(fmt,var) 
 #   define RETURN(e,c,m,...) if(c){kprintf("Error "m"\n");__VA_ARGS__;return e;}
 #   define OPENR(f) open (f)
 #   define OPENW(f)
 #   define PRINT(fmt,...)
 #   include <common/cstd.h>    
 #endif

/**
 * linked list to handle words
 * words should be separated by
 * "blank" chars
 * 
 * ls -la somefile
 * "ls" -> "-la" -> "somefile" -> NULL
 * 
 * do i need word types ??
 * * args
 * * program
 * * files
 * * options 
*/
struct wordlist {
    struct wordlist *nxt;
    char *word;
};

enum stmt_type {
    NULL_TYPE,          /*           sentinel */
    WHILE_TYPE,         /*        while block */
    IF_TYPE,            /*           if block */
    PIPELINE_TYPE,      /*           pipeline */
    BUILT_IN_TYPE,      /*       built-in fux */
    EXEC_TYPE,          /*       exec program */
    EXPR_TYPE,          /*         expression */
    ENV_ASSIGN_TYPE     /* env var assignment */
};

/* struct for a generic statement */
struct stmt {
    enum stmt_type t; /* simple or compound ? */

    union {
        /* compound stmts */
        struct if_stmt *if_stmt;
        struct while_stmt *while_stmt;
        struct expr *expr;
        /* simple stmts */
        struct wordlist *simple_stmt;
    } stmt;

    struct stmt *nxt;
};



enum expr_type {
    NULL_EXPR,
    AND_OP,
    OR_OP,

    PLUS_OP,
    MINUS_OP,
    MULT_OP,
    DIV_OP,

    EQ_OP,
    NEQ_OP,

    /* unary operators */
    ASSIGN_OP,

    LT_OP,
    GT_OP,
    LEQ_OP,
    GEQ_OP,

    NOT_OP,

    /* data types */
    INT_EXPR,
    WORD_EXPR,
    STMT_EXPR,
};



/**
 * this is a binary expression
 * maybe it's a good idea to 
 * handle this as a bintree...
 * */ 
struct expr {
    enum expr_type t;       /*   the operation + * - && ... */

    union {                 /*           value of this node */
        int v;              /*                        value */
        char *word;         /*                     variable */
        struct stmt *stmt;  /*     statement maybe not used */ 
        struct expr *e[2];  /* expression at left and right */
    } v;
};

/**
 * you're doing this because you could
 * get to the situation where a condition
 * could change so you will need to eval
 * more than one time (on a loop, e.g.)
 * remember this is meant to be continued 
 * after you.
 */

 /**
  * \brief if statement structure
  */
struct if_stmt {
    struct stmt *condition;    /*                   condition */
    struct stmt *branch[2];    /* true(1) and false(0) branch */
};

/**
 * \brief while statement structure
 * 
 */
struct while_stmt {
    struct stmt *condition;     /* condition */
    struct stmt *execute;       /*   execute */
};


/* typedefs */

typedef struct wordlist wordlist_s;
typedef struct stmt stmt_s;
typedef struct if_stmt if_stmt_s;
typedef struct while_stmt while_stmt_s;
typedef struct expr expr_s;

typedef enum expr_type expr_type_e;
typedef enum stmt_type stmt_type_e;

/* execution */

int execute_stmt(stmt_s *stmt);
int execute_while_stmt(while_stmt_s *wstmt);
int execute_if_stmt(if_stmt_s *istmt);
int execute_built_in(stmt_s *bstmt);
int execute_cmd(stmt_s *cstmt);

/* expr outils */

/**
 * \brief   create a struct expression
 * \return  the pointer to the newly 
 *          struct expr. NULL on error
 * \note    the type is set to NULL_EXPR. 
 *          this function call malloc and 
 *          the memory must be freed with
 *          `expr_destroy()`
 */
struct expr *expr_create(void);

/**
 * \brief   destroy a struct expr
 * \param   victim the pointer to the struct
 *          expr.
 * \note    this function frees the 
 *          memory allocated for a struct 
 *          expr and the included structures 
 *          and may call the methods to 
 *          destroy wordlist and stmts
 */
void expr_destroy(expr_s *victim);

/**
 * \brief   set expression as operation
 * \param   expr the expr_s to set
 * \param   op the operation
 * \param   l left operand as expr_s
 * \param   r right operand as expr_s
 * \return  1 on success. 0 on failure.
 * \note    in the case of unary operations
 *          the left operand must be set
 * 
 */
int expr_set_op(expr_s *expr, expr_type_e op, expr_s *l, expr_s *r);

/**
 * \brief   set expr_s as variable
 * \param   expr the expr_s
 * \param   w the name of the variable as a wordlist
 * \return  1 on success. 0 on failure
 * \note    this function will create a copy of w 
 *          which will be freed on the call 
 *          to `expr_destroy()`
 */
int expr_set_var(expr_s *expr, const char *w);

/**
 * \brief   set expr_s as integer
 * \param   expr the expr_s
 * \param   v the int
 * \return  1 on success. 0 on failure
 */
int expr_set_int(expr_s *expr, int v);

/**
 * \brief   set expr_s as a stmt
 * \param   expr the expr_s
 * \param   stmt the stmt
 * \return  1 on success. 0 on failure
 * \note    stmt will be freed on the call 
 *          to `expr_destroy()`.
 */
int expr_set_stmt(expr_s *expr, stmt_s *stmt);

/**
 * \brief prints an expression structure
 */
void expr_print(expr_s *expr);

/**
 * \brief   this function evaluates an expression
 */
char *eval_expr(expr_s *expr);
int eval_expr_int(expr_s *expr);

/* wordlist outils */

/**
 * \brief   function to create a wordlist struct
 * \return  A pointer to the newly wordlist. NULL on error.
 * \note    if succeed nxt will be pointing to NULL.
 */
struct wordlist *wordlist_create(void);

/**
 * \brief   frees the wordlist until it finds nxt == NULL
 * \param   wordlist the "head" of the list
 * \note    this function will also free word. This function
 *          will the destroy the wordlist from the wordlist
 *          to the end of the list. If there's something 
 *          behind, it will not be destroyed.
 */
void wordlist_destroy(struct wordlist *wordlist);

/**
 * \brief   helper function to print a wordlist until it finds
 *          nxt == NULL
 * \param   wordlist the head of the list
 * \note    just for debug :P
 */
void wordlist_print(struct wordlist *wordlist);

/* maker functions */

/**
 * \brief   creates a wordlist struct with word=str
 * \param   str the head of the list
 * \return  A pointer to the newly wordlist. NULL on error.
 * \note    this function will alloc memory for word and then
 *          copy the content of str. be sure to make a free to
 *          str if you don't need it any longer.
 */
struct wordlist *make_wordlist(const char *str);

/**
 * \brief   creates a wordlist struct and puts it in the head 
 *          of the list
 * \param   l head of the list
 * \param   str the string will be put into the word attribute 
 *          of the newly wordlist
 * \return  A pointer to the newly wordlist. NULL on error.
 * \note    this function uses make_wordlist to create the new
 *          wordlist. If this function fails, l will be unaltered
 */
struct wordlist *wordlist_pushfront(struct wordlist * l, const char *str);


/* statement related stuff */

/**
 * \brief   function to create a generic stmt struct
 * \return  A pointer to the newly generic stmt. NULL on error.
 * \note    if succeed nxt will be pointing to NULL and the 
 *          type will be set to NULL_TYPE
 */
stmt_s *stmt_create(void);

/**
 * \brief   destroy a stmt
 * \param   victim the stmt to destroy
 * \note    this fuction destroys only the victim and 
 *          will not touch the next stmt. On the contrary, 
 *          it will erase all under the block if it's a flow control
 *          statement. This function may call while_stmt_destroy and
 *          if_stmt_destroy and itself.
 *          If stmt is NULL, this function has no effect.
 */
void stmt_destroy(stmt_s *victim);

/**
 * \brief   function to create an if_stmt struct
 * \return  A pointer to the newly generic if_stmt. NULL on error.
 * \note    if_case and else_case will be pointing to NULL
 */
if_stmt_s *if_stmt_create(void);

/**
 * \brief   destroy an if_stmt
 * \param   victim the if_stmt to destroy
 * \note    this fuctions destroys the victim and all the 
 *          conditional block it represents.
 *          If stmt is NULL, this function has no effect.
 *          this function may call stmt_destroy().
 *          note that the nxt attribute will remain unmodified.
 */
void if_stmt_destroy(if_stmt_s *victim);

/**
 * \brief   function to create an while_stmt struct
 * \return  A pointer to the newly generic while_stmt. NULL on error.
 * \note    condition and execute will be pointing to NULL
 */
while_stmt_s *while_stmt_create(void);

/**
 * \brief   destroy an while_stmt
 * \param   victim the while_stmt to destroy
 * \note    this fuctions destroys the victim and all the 
 *          conditional block it represents.
 *          If stmt is NULL, this function has no effect.
 *          this function may call stmt_destroy().
 *          note that the nxt attribute will remain unmodified.
 */
void while_stmt_destroy(while_stmt_s *victim);

/**
 * \brief   set up a stmt to hold an expression
 * \param   stmt the stmt
 * \param   expr the expression
 * \return  1 on success 0 on error
 */
int stmt_set_expr(stmt_s *stmt, expr_s *expr);

/**
 * \brief   set up a stmt to hold a simple statement
 * \param   stmt the stmt
 * \param   w wordlist of the simple statement
 * \param   t type of the simple statement {BUILT-IN, EXEC, ENV}
 * \return  returns 1 on success and 0 on error.
 * \note    the wordlist won't be hardcopied to simple_stmt. On 
 *          the other hand, only the reference will be copied and 
 *          the memory allocated for it will be liberated by stmt_destroy()
 */
int stmt_set_simple(stmt_s *stmt, wordlist_s *w, enum stmt_type t);

/**
 * \brief   this functions set the stmt up to hold a if_stmt in stmt lol
 * \param   t_case the true case of the new if_stmt
 * \param   f_case the false case of the new if_stmt
 * \return  1 on succeed. 0 on error
 * \note    this function will call if_stmt_create()
 */
int stmt_set_if_stmt(stmt_s *stmt, stmt_s *cond, stmt_s *t_case, stmt_s *f_case);

/**
 * \brief   this functions set the stmt to hold a while_stmt in stmt lol
 * \param   cond condition
 * \param   t_case the stmt to execute if cond is true
 * \return  1 on succeed. 0 on error
 * \note    this function will call while_stmt_create()
 */
int stmt_set_while_stmt(stmt_s *stmt, stmt_s *cond, stmt_s *t_case);

/**
 * \brief   sets the statement to be executed after another
 * \param   stmt a statement
 * \param   nxt the next statement
 * \return  1 on succeed. 0 on error
 * \note    
 */
int stmt_set_next(stmt_s *stmt, stmt_s *nxt);


/**
 * \brief   print some stmt info
 * \param   stmt 
 */
void stmt_print(stmt_s *stmt);

/* booling :P mdr lol */
/**
 * \brief   this function transforms an integer into 
 *          boolean values as the shell interprets them
 *          0     -> true
 *          not 0 -> false
 * \param   v int or char
 * \return  int value corresponding to the value
 * \note    
 */
int i2kbool(int v);

/**
 * \brief   this function transforms an integer representing 
 *          a kshell boolean into an integer representing a 
 *          a c bool
 *          0     -> true
 *          not 0 -> false
 * \param   v int or char
 * \return  int value corresponding to the value
 * \note    
 */
int kbool2i(int v);

#endif

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
