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
                                                                                                   
\*------------------------------------------------------------------------------------------------*/

#ifndef _PSF_H_
#define _PSF_H_

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

#include <common/list.h>

//--------------------------------------------------------------------------------------------------
// FLAGS Value (1 octet)
//--------------------------------------------------------------------------------------------------

    // Type 5 bit => 32 type possible but that's a lot, we can shrink it later
#define FILE    (1 << 4)
#define DIR     (1 << 5)

    // Rights
#define R       (1 << 1)
#define W       (1 << 2)
#define X       (1 << 3)
#define RWX     R|X|W
#define RW      R|W

//--------------------------------------------------------------------------------------------------
// DATA Info (not stored it will be well known)
//--------------------------------------------------------------------------------------------------
#define DATA_SIZE 512
#define DATA_TYPE char*

//--------------------------------------------------------------------------------------------------
// NAME Specification (39 octet)
//--------------------------------------------------------------------------------------------------
#define NAME_SIZE 39

//--------------------------------------------------------------------------------------------------
// ERROR VALUE (need to use import => erase it after complition problem sloved)
//--------------------------------------------------------------------------------------------------
#define EINVAL -1

//--------------------------------------------------------------------------------------------------
// Peudo File Systeme specification
//--------------------------------------------------------------------------------------------------

/**
 * \brief 
 * \todo    faire une breve description Lili
 * \remarks le nom des champs est toujours sujet a debat
 */
struct pfs{
    char flags;         /**< \var flags give a type for different usage */
    char name[39];      /**< \var name of file or directory. */
    void* data;         /**< \var data block of data if not a directory*/
    int size;           /**< \var size of data */
    list_t root;        /**< \var root parent directory field */
    list_t brothers;    /**< \var brother object in the same directory */
};

/**
 * \brief etant donner nom rend la structure qui associé pour l'accès au dossier
 */
struct pfs* opendir(const char *pathname);

/**
 * \brief ouvre un fichier etant donner le chemin
 * \var flags ne sert a rien car on n'a pas de macanisme de gestion de droits
 * \var pathname donne le chemin absolu du fichier
 * 
 * \remarks On a pas de working directory .
 */
struct pfs* pfs_open(const char *pathname, int flags);


/**
 * \brief flush the buffer in the disk
 */
long write(int fd, const void *buf, size_t count);

int readdir(unsigned int fd, unsigned int count);

long read(int fd, void *buf, size_t count);

#endif // _PSF_H_