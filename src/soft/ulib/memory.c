/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     ulib/memory.c
  \author   Franck Wajsburt
  \brief    User memory allocator

\*------------------------------------------------------------------------------------------------*/

#include <libc.h>

//------------------------------------------------------------------------------- private definition

static size_t CacheLineSize;    // cache line size set by malloc_init

typedef struct block_info_s {   // small structure always put at the beginning of each blocks
    unsigned full:1;            // 1 full, 0 free (means empty)
    unsigned magic:7;           // MAGIC_HEAP : magic number to check the corruption
    unsigned size:24;           // Number of block_info to the next block_info
} block_info_t;

static struct heap_s {          // user Heap
    block_info_t *beg;          // Heap beginning
    block_info_t *end;          // Heap end
} Heap;

// C Macros to align a pointer p to the current cache line address or the next one
// For example, let the CacheLineSize is 0x10 Bytes (4 int), then
// if p = 0x76543214 then LINE_FLOOR(p) = 0x76543210 and LINE_CEIL(p) = 0x76543220

#define LINE_FLOOR(p)   (block_info_t *)FLOOR((size_t)(p),CacheLineSize)
#define LINE_CEIL(p)    (block_info_t *)CEIL((size_t)(p),CacheLineSize)
#define BINFO_SZ        sizeof(block_info_t)

static void* try_malloc (size_t size)
{
    size = CEIL (size+BINFO_SZ, CacheLineSize);             // true required size in bytes
    size = size / sizeof (block_info_t);                    // in the heap size is in block_info_t
    block_info_t *oldnext, *newnext, *new;
    for (new = Heap.beg;                                    // from the beginning of the Heap
        (new < Heap.end) && (new->full||(new->size<size));  // while end not reached and no space
        new += new->size) {}

    if (new >= Heap.end) return NULL;                       // end reached without finding space

    new->full = 1;                                          // space found, we put the block
    oldnext = new + new->size;                              // next block address before the cut
    newnext = LINE_CEIL(new+size);                          // find the new next block

    if (newnext != oldnext) {                               // if we need to cut the find block
        new->size = newnext - new;                          // new size of current block
        new->magic = MAGIC_HEAP;                            // to try detect Heap corruption
        newnext->size = oldnext - newnext;                  // new size of remaining space
        newnext->full = 0;                                  // that is free space
        newnext->magic = MAGIC_HEAP;                        // to try detect Heap corruption
    }
    return (void *)(new + 1);                               // the allocated block after block_info
}

static void merge (block_info_t * ptr)
{
    for (; ptr != Heap.end; ptr += ptr->size)               // try to merge all free blocks from ptr
        if (ptr->full == 0)                                 // if current block is free
            for (block_info_t * next = ptr + ptr->size;     // find next one
                ((next != Heap.end) && (next->full == 0));  // while not end found a free block
                next = ptr + ptr->size)                     // go to next block
                ptr->size += next->size;                    // merge current block with first free
}

//-------------------------------------------------------------------------------- public definition

void * sbrk (int incr)
{
    void *r = (void*)syscall_fct(incr,0,0,0,SYSCALL_SBRK);  // ask for heap boundary change
    return r;
}

void malloc_init (void *beg)
{
    CacheLineSize = cachelinesize();                        // need to know the cache line size
    int *end = sbrk (4* PAGE_SIZE);                         // try to get 4 pages (16ko)

    if (end == (int *)-1) exit (2);                         // if impossible exit the app
    Heap.beg = LINE_FLOOR (beg);                            // address of the first BLOCK
    Heap.end = LINE_FLOOR (end);                            // address of the boundary
    Heap.beg->full = 0;                                     // empty at the begining
    Heap.beg->magic = MAGIC_HEAP;                           // to try detect Heap corruption
    Heap.beg->size = Heap.end - Heap.beg;                   // size of free space
    CacheLineSize = cachelinesize();                        // ask the kernel for cache line size
}

void * malloc (size_t size)
{
    block_info_t *ptr = try_malloc (size);                  // Search for a block
    if (ptr == NULL) {                                      // if no free space
        merge (Heap.beg);                                   // merge all free blocks from beginning
        ptr = try_malloc (size);                            // try again to find a block
    }
    if (ptr == NULL) errno = ENOMEM;
    return ptr;                                             // return what you have found
}

void *calloc(size_t n, size_t size)
{
    size_t total = n * size;                                // total number of char
    unsigned *ptr = malloc (total);                         // try to allocate (always word-aligned)
    if (!ptr) return NULL;                                  // no more space
    for (int i = (total - 1) / 4; i >= 0; i--)              // clear the zone (the last word may be 
        ptr[i] = 0;                                         // imcomplete, it is allocated
    return (void *) ptr;                                    // return a void *
}

char * strdup (const char * str)
{
    if (str == NULL) return NULL;                           // Avoid NULL input 
    size_t len = strlen(str) + 1;                           // Include null terminator
    char *copy = (char *) malloc(len);                      // Allocate memory 
    if (copy == NULL) errno = ENOMEM;
    memcpy(copy, str, len);                                 // Copy the string, including '\0'
    return copy;                                            // Return the allocated copy
}


void free (void *ptr)
{
    block_info_t *info = (block_info_t *)ptr - 1;           // block_info is just before ptr
    if (!ptr || !info->full || (info->magic != MAGIC_HEAP)) // pt NULL OR segment free OR not MAGIC
        exit(1);                                            // memory corrupted
    info->full = 0;                                         // erase the full flag
}

void malloc_print (int level)
{
    block_info_t *ptr;
    fprintf (0, "------------ %p ------------\n", Heap.beg);
    for (ptr = Heap.beg; ptr < Heap.end; ptr += ptr->size){ // browse all blocks
        fprintf (0, " %p %d %s  [ %x\t- %x\t] = %d\n",
            ptr, ptr->magic,
            (ptr->full) ? "full" : "free",                  // print if block is full or free
            BINFO_SZ*(size_t)(ptr - Heap.beg),              // print block begin position
            BINFO_SZ*(size_t)((ptr + ptr->size) - Heap.beg),// print block end position
            BINFO_SZ*(size_t)(ptr->size));                  // print block size
        while (ptr->size == 0);
    }
    fprintf (0, "------------ %p ------------\n", Heap.end);
}

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
