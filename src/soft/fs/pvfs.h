/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-19
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     pvfs.c
  \author   Lili Zheng  
  \brief    pvfs is a pseudo file system made in order to test the kshell,
            while there is no real file system 
            this file system was build when there wasn't any proc structure and disk controller

  State : done but not fully tested

   ┌===================┐                                                                           
   ;For example        ;                                                                           
   ;                   ;                                                                           
   ;  /                ;                                                                           
   ;  ├── data         ;                                                                          
   ;  │   └── read.txt ;                                          ┌──────────┐                     
   ;  └── etc          ;                                          │ FILE     │                     
   ;                   ;                                          ├──────────┤                     
   └===================┘                                          │ read.txt │                     
                              ┌──────────┐    ┌──────────┐        ├──────────┤                     
                              │ DIR      │    │ DIR      │        │┌────────┐│                     
                              ├──────────┤    ├──────────┤        ││ root   ││                     
                              │ 'etc'    │ ...│ 'data'   │        │└────────┘│                     
              ┌──────────┐    ├──────────┤  ▲ ├──────────┤        ├──────────┤                     
              │ DIR      │    │┌────────┐│  | │┌────────┬┼────────►┌────────┬┼──►...               
              ├──────────┤    ││ root   ││  | ││ root   ││        ││ brother││                     
              │ '/'      │    │└────────┘│  ---└────────┘◄────────┼┴────────┘│                     
              ├──────────┤    ├──────────┤    ├──────────┤        ├──────────┤  ┌──────────┐       
           ┌──►┌────────┬┼────►┌────────┬┼────►┌────────┬┼──┐     │ Data   ──┼─►│          │       
           │  ││ root   ││    ││ brother││    ││ brother││  │     └──────────┘  │ 4ko ...  │       
           │┌─┼┴────────┘◄────┼┴────────┘◄────┼┴────────┘◄─┐│                   │          │       
           ││ ├──────────┤    ├──────────┤    ├──────────┤ ││                   └──────────┘       
           ││ │┌────────┐│    │ NULL     │    │ NULL     │ ││                                      
           ││ ││ brother││    └──────────┘    └──────────┘ ││                                      
           ││ │└────────┘│                                 ││                                      
           ││ ├──────────┤                                 ││                                      
           ││ │ NULL     │                                 ││                                      
           ││ └──────────┘                                 ││                                      
           │└──────────────────────────────────────────────┘│                                      
           └────────────────────────────────────────────────┘      (made with www.asciiflow.com) 

   \remarks It represent in linux an inode but here the pvfs know his parent directory
            this implies that we won't have hard link neither symbolic link.asm
            Directories data are NULL because we don't need to flush/extract from a disk partition

            For the API interface i tried as much as possible to be like linux.

            Since I was not confortable with File system Franck helped a lot!
\*------------------------------------------------------------------------------------------------*/
#ifndef _PSF_H_
#define _PSF_H_

#   include <common/cstd.h>
#   include <common/usermem.h>   
#   include <common/errno.h> 
#include <common/list.h>

//==================================================================================================
// PVFS definition
//==================================================================================================

// FLAGS Value (1 byte)

   // Type 5 bit => 32 values
#define FILE_T      (1 << 3)
#define DIR_T       (2 << 3)
#define EXEC_T      (3 << 3)
#define STDIN_T     (4 << 3)
#define STDOUT_T    (5 << 3)
#define STDERR_T    (6 << 3)
#define IS_TYPE(flag, type) (((flag) & 0xF8) == (type) )

   // File Rights
#define R       (1 << 0)
#define WRITE   (1 << 1)
#define X       (1 << 2)
#define RWX     R|X|WRITE
#define RW      R|WRITE

// NAME Specification (35 byte)
#define NAME_SIZE 35
#define PATH_MAX 4096
#define PAGE_SIZE 4096


typedef struct pfs_s{
   char flags;             ///< flags give a type for different usage 
   char name[NAME_SIZE];   ///< name of file or directory. 
   void* data;             ///< data block of data if not a directory
   int size;               ///< size of data 
   list_t root;            ///< root parent directory field 
   list_t brothers;        ///< brother object in the same directory 
   struct pfs_s * parent;  ///< parent directory
   
} pfs_t;

/**
 * This function initialise 
 */
void pvfs_init (void);

int open(const char* pathname, int flags);

int close(int fd);

int file_write(int fd, char *buf, unsigned count);

int file_read(int fd, char *buf, unsigned count);

int unlink(const char *pathname);

//==================================================================================================
// DIRENT definition
//==================================================================================================
// \remark DIR need to be accessible from user space

#define DIR struct dirent_s
struct dirent_s {
    char  d_name[PATH_MAX]; 
    pfs_t * root; ///< pointer to the current directory
    pfs_t * item; ///< pointer to the item in current directory
};
typedef struct dirent_s dirent_t;

/**
 * \brief give access to a directory
 */
// DIR* opendir(char *pathname);

// int closedir(DIR* dirp);

struct dirent_s* readdir(DIR* dirp);

void rewinddir(DIR *dirp);

int chdir(const char *pathname);

int rmdir(const char *pathname);

// lack the system call getdent(fd, dirent*, count)



//==================================================================================================
// file_s definition for open file
//==================================================================================================
typedef struct file_s{
   struct pfs_s * file;
   char flags; 
      int (*read)(int fd, char *buf, unsigned count);
      int (*write)(int fd, char *buf, unsigned count);
   int type;
   int ref;
} file_t;

// Files Descriptor are defined in /common/usermem.h
//    #define MAX_O_FILE 64 

int dup2(int oldfd,int newfd);

#endif // _PSF_H_
/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
