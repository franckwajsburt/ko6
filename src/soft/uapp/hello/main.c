/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     hello/main.c
  \author   Franck Wajsburt
  \brief    first program

\*------------------------------------------------------------------------------------------------*/

#include <libc.h>

int main (void)
{
    char name[64];
    fprintf (1, Banner_ko6);
    fprintf (1, "Hello world!\n");
    fprintf (1, "What's your name? ");
    fgets (name, sizeof(name), 1);
    name[ strlen(name)-1 ]=0;
    fprintf (2, "Hello %s!\n", name);
    fprintf (3, "The end\n");
    return 0;
}

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
