#ifndef KSHELL_H
#define KSHELL_H
/**
 * You're gonna need a returning value
 * convention
 * 
 * 0 -> ok yippee
 * other -> no ok no yippee
 * 
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
    WHILE_TYPE,          /* while block   */
    IF_TYPE,             /* if block      */
    BUILT_IN_TYPE,       /* built-in fux  */
    EXEC_TYPE,           /* exec program  */
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
struct if_stmt {
    struct expr *condition;    /* condition */
    struct stmt *true_case;    /* true branch */
    struct stmt *false_case;   /* false branch */
};

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
struct wordlist *wordlist_create(void);
void wordlist_destroy(struct wordlist *wordlist);
void wordlist_print(struct wordlist *wordlist);

/* maker functions */

struct wordlist *make_wordlist(const char *str);
struct wordlist *wordlist_pushfront(struct wordlist * l, char *str);

#endif