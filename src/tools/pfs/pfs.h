/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date        2025-03-16
  | / /(     )/ _ \     \copyright   2021 Sorbonne University
  |_\_\ x___x \___/                  https://opensource.org/licenses/MIT

  \file     /tools/pfs/pfs.h
  \author   Franck Wajsburt
  \brief    pfs a pseudo file system in order to test the kshell, 
  
  State: building

  NB:   it is a naive version,
        to replace de real file system while it is not ready
                                                                                                        
   ┌===================┐                                                                           
   .For example        .                                                                           
   .                   .                                                                           
   ;  /                .                                                                           
   ;  ├── data         .                                                                           
   ;  │   └── read.txt .                                          ┌──────────┐                     
   ;  └── etc          .                                          │ FILE     │                     
   ;                   .                                          ├──────────┤                     
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
           └────────────────────────────────────────────────┘     
           
    \remarks it's ok if it don't define everything, the worse is to have incompatible function.
                                                                                                   
\*------------------------------------------------------------------------------------------------*/

#ifndef _PSF_H_
#define _PSF_H_

#ifdef _HOST_
#   include <stdio.h>
#   include <stdlib.h>
#   include <stdint.h>
// #   include <fcntl.h>
#   include <stddef.h> 
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
#   include <common/usermem.h>    
#endif

#include <common/list.h>

//--------------------------------------------------------------------------------------------------
// FLAGS Value (1 octet)
//--------------------------------------------------------------------------------------------------

    // Type 5 bit => 32 type possible but that's a lot, we can shrink it later
#define FILE_T    (1 << 4)
#define DIR_T     (1 << 5)
#define EXEC_T    (1 << 6)

    // Rights
#define R       (1 << 1)
#define W       (1 << 2)
#define X       (1 << 3)
#define RWX     R|X|W
#define RW      R|W

//--------------------------------------------------------------------------------------------------
// DATA Info (not stored it will be well known)
//--------------------------------------------------------------------------------------------------
#define DATA_SIZE 4096
#define DATA_TYPE char*

//--------------------------------------------------------------------------------------------------
// NAME Specification (39 octet)
//--------------------------------------------------------------------------------------------------
#define NAME_SIZE 35
#define PATH_MAX 4096
#define PAGE_SIZE 4096

//--------------------------------------------------------------------------------------------------
// ERROR VALUE (need to use import => erase it after complition problem sloved)
//--------------------------------------------------------------------------------------------------
#define EINVAL -1

//--------------------------------------------------------------------------------------------------
// Descriptor : {File, Dir}
//--------------------------------------------------------------------------------------------------
#define MAX_FD 64 //personnal choice
#define MAX_DD 64 //TODO change give his own header file

//--------------------------------------------------------------------------------------------------
// Peudo File Systeme specification
//--------------------------------------------------------------------------------------------------

/**
 * \brief 
 * \todo    faire une breve description Lili
 * \remarks le nom des champs est toujours sujet a debat
 */
struct pfs_s{
    char flags;         /**< \var flags give a type for different usage */
    char name[NAME_SIZE];      /**< \var name of file or directory. */
    void* data;         /**< \var data block of data if not a directory*/
    int size;           /**< \var size of data */
    list_t root;        /**< \var root parent directory field */
    list_t brothers;    /**< \var brother object in the same directory */
    struct pfs_s * parent;
    
};
typedef struct pfs_s pfs_t;

int open(const char* pathname, int flags);

int close(int fd);

int _write(int fd, const void *buf, int count);

int _read(int fd, void *buf, int count);

//--------------------------------------------------------------------------------------------------
// Dirent in User Space (need to have his own header not sure)
//--------------------------------------------------------------------------------------------------
#define DIR int

struct dirent_s {
    char  d_name[PATH_MAX]; 
    pfs_t * root; ///< pointer to the current directory
    pfs_t * item; ///< pointer to the item in current directory
};
typedef struct dirent_s dirent_t;

/**
 * \brief etant donner nom rend la structure qui associé pour l'accès au dossier
 */
DIR* opendir(char *pathname);

int closedir(DIR* dirp);

struct dirent_s* readdir(DIR* dirp);

void rewinddir(DIR *dirp);

#endif // _PSF_H_