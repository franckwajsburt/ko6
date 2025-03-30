/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date        2025-03-30
  | / /(     )/ _ \     \copyright   2021 Sorbonne University
  |_\_\ x___x \___/                  https://opensource.org/licenses/MIT

  \file     /tools/kshell/kshell.h
  \author   Marco Leon
  \brief    here you'll find the headers and fux declarations regarding the 
            kshell. You may see some notes, you can ignore them, they are 
            probably lame ideas.

  State :   building
  
  NB :      :P


\*------------------------------------------------------------------------------------------------*/
#ifndef KSHELL_H
#define KSHELL_H
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
    NULL_TYPE,           /*      sentinel */
    WHILE_TYPE,          /*   while block */
    IF_TYPE,             /*      if block */
    PIPELINE_TYPE,       /*      pipeline */
    BUILT_IN_TYPE,       /*  built-in fux */
    EXEC_TYPE,           /*  exec program */
};

/* struct for a generic statement */
struct stmt {
    enum stmt_type t; /* simple or compound ? */
    union {
        /* compound stmts */
        struct if_stmt *if_stmt;
        struct while_stmt *while_stmt;
        /* simple stmts */
        struct wordlist *simple_stmt;
    } stmt;
    struct stmt *nxt;
};

/**
 * this is a binary expression
 * maybe it's a good idea to 
 * handle this as a bintree...
 * depending on the type, we 
 * could decide which branch of 
 * the bintree it's gonna be 
 * executed first
 * 
 * maybe i won't use this
 * */ 
struct expr {
    char type;          /* the operation + * - && ... */
    struct expr *lexpr;
    struct expr *rexpr;
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
    struct expr *condition;    /* condition */
    struct stmt *true_case;    /* true branch */
    struct stmt *false_case;   /* false branch */
};

/**
 * \brief while statement structure
 * 
 */
struct while_stmt {
    struct expr *condition;     /* condition */
    struct stmt *execute;       /* execute */
};


/* typedefs */

typedef struct wordlist wordlist_s;
typedef struct stmt stmt_s;
typedef struct if_stmt if_stmt_s;
typedef struct while_stmt while_stmt_s;
typedef struct expr expr_s;


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
 * \brief   set up a stmt to hold a simple statement
 * \param   w wordlist of the simple statement
 * \param   t type of the simple statement {BUILT-IN, EXEC}
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
int stmt_set_if_stmt(stmt_s *stmt, expr_s *cond, stmt_s *t_case, stmt_s *f_case);

/**
 * \brief   this functions set the stmt to hold a while_stmt in stmt lol
 * \param   cond condition
 * \param   t_case the stmt to execute if cond is true
 * \return  1 on succeed. 0 on error
 * \note    this function will call while_stmt_create()
 */
int stmt_set_while_stmt(stmt_s *stmt, expr_s *cond, stmt_s *t_case);

/**
 * \brief   sets the next statement to be executed
 * \param   stmt a statement
 * \param   nxt the next statement
 * \return  1 on succeed. 0 on error
 * \note    
 */
int stmt_set_next(stmt_s *stmt, stmt_s *nxt);

#endif