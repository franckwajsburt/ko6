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

#include <common/htopen.h>
#include <stmt.h>
#include <varenv.h>

/* execution */

/**
 * \brief   this function evaluates an expr structure
 *          which has to be initialized
 * \param   expr expr to execute
 * \return  the evaluation
 * \note    this function may call itself and 
 *          `kshell_stmt_execute()`recursively
 */
int kshell_expr_eval(expr_s *expr);
int kshell_stmt_execute(stmt_s *stmt);
int kshell_while_stmt_execute(while_stmt_s *wstmt);
int kshell_if_stmt_execute(if_stmt_s *istmt);
int kshell_built_in_execute(stmt_s *bstmt);
int kshell_cmd_execute(stmt_s *cstmt);

char *kshell_parse_assing(const char *assgn);

int kshell_ls(wordlist_s *args);
int kshell_cat(wordlist_s *args);
int kshell_echo(wordlist_s *args);
int kshell_export(wordlist_s *args);
int kshell_kshell(wordlist_s *args);
int kshell_cd(wordlist_s *args);
int kshell_pwd(wordlist_s *args);
int kshell_top(wordlist_s *args);
int kshell_kill(wordlist_s *args);
int kshell_kvar(wordlist_s *args);
int kshell_su(wordlist_s *args);

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

/**
 * \brief   this function records an env variable
 * \param   n name of the variable
 * \param   v value of the variabel
 * \param   flags
 */
int kshell_set_varenv(char *n, const char *v, int flags);

void kshell_print_env();

int kshell_unset_varenv(hto_t *ht, char *name);

#endif

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
