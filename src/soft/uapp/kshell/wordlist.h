#ifndef _KSHELL_WORDLIST_H_
#define _KSHELL_WORDLIST_H_

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
#   define PRINT(fmt,...) printf(fmt, ##__VA_ARGS__)
#else
#   define MALLOC(s) malloc(s)
#   define FREE(s) free(s)
#   define P(fmt,var) 
#   define RETURN(e,c,m,...) if(c){kprintf("Error "m"\n");__VA_ARGS__;return e;}
#   define OPENR(f) open (f)
#   define OPENW(f)
#   define PRINT(fmt,...)
#   include <common/cstd.h>    
#   include <ulib/memory.h>
#endif

typedef struct wordlist wordlist_s;

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


typedef struct wordlist wordlist_s;


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


#endif