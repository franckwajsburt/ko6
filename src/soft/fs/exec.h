/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-27
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     fs/exec.h
  \author   Franck Wajsburt
  \brief    program loader and process starter

  This module provides the fork_exec() function, responsible for loading an ELF executable into 
  memory, creating a new process, and starting its first thread at the main() function entry point.

\*------------------------------------------------------------------------------------------------*/

#ifndef _EXEC_H_
#define _EXEC_H_

/**
 * \brief Loads and starts a new program in memory.
 *        This function creates a new process, loads an ELF executable file into memory, 
 *        sets up the entry point, and creates the first thread for the program's main function.
 * \param path  The path to the ELF executable file.
 * \param argv  Array of argument strings, terminated by a NULL pointer.
 * \param envp  Array of environment strings, terminated by a NULL pointer (optional).
 * \return The new process PID on success, or a negative error code on failure.
 */
int fork_exec(const char *path, const char *argv[], const char *envp[]);

#endif//_EXEC_H_

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
