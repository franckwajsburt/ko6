#ifndef _KSHELL_VARENV_H_
#define _KSHELL_VARENV_H_

/* env variables */

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
#   define MALLOC(s) kmalloc(s)
#   define FREE(s) kfree(s)
#   define P(fmt,var) 
#   define RETURN(e,c,m,...) if(c){kprintf("Error "m"\n");__VA_ARGS__;return e;}
#   define OPENR(f) open (f)
#   define OPENW(f)
#   define PRINT(fmt,...)
#   include <common/cstd.h>    
#endif

#define ATTR_EXPORTED 0x1
#define KSHELL_TEST_ATTR(attr, mask) (attr)&(mask)

typedef struct varenv varenv_s;

struct varenv {
    char * v;          /*      value */
    int attr;          /* attributes */
};

/**
 * \brief   allocates memory for a 
 *          varenv structure
 */
struct varenv *varenv_create(void);

/**
 * \brief   destroys a struct varenv
 * \param   victim victim to destroy
 */
void varenv_destroy(struct varenv *victim);

/**
 * \brief   set the value of the var
 * \param   var the initialized env var
 * \param   v the string to be stored than var
 * \return  1 on succed, 0 on error.
 * \note    the attribute of the structure 
 *          will be UPDATED with the new v. The 
 *          previous attribute will be freed.
 */
int varenv_value_set(struct varenv *var, const char * v);

/**
 * \brief   sets the attributes of the var given a mask
 * \param   var the initialized env var
 * \param   mask the attributes. must be set 
 *          using the macros
 * \return  1 on succed, 0 on error.
 */
int varenv_attr_set(struct varenv *var, int mask);

/**
 * \brief   unsets the attributes of the var given a mask
 * \param   var the initialized env var
 * \param   mask the attributes. must be unset 
 *          using the macros.
 * \return  1 on succed, 0 on error.
 */
int varenv_attr_unset(struct varenv *var, int mask);

#endif
