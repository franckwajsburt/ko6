/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     common/htopen.c
  \author   Franck Wajsburt
  \brief    see comment in common/htopen.h

  FIXME should use a bitmap to know if a entry is empty or not, not a forbidden key KEYFREED
  FIXME key type should be an union to avoid cast :
        typedef union { void * v; unsigned long u; char *s } hto_key_t;

\*------------------------------------------------------------------------------------------------*/

#define KEYFREED ((void *)0xF0000001)               ///< key used when a slot is freed 

#include <htopen.h>
#include <errno.h>

#ifdef _KERNEL_                                     // if it is for the kernel
#   include <kernel/klibc.h>
#   define PAGE_SIZE    4096
#   define MALLOC       kmalloc                     // allocates in the slab allocator
#   define STRDUP       kstrdup                     // allocates a new key (when it is a string)
#   define FREE(k)      kfree(k)                    // free a key (when it is a string)
#   define PRINT(...)   kprintf(__VA_ARGS__) 
#   define MALLOC_P(l)  
#else                                               // if it is for the user
#   define MALLOC       malloc                      // allocates in the libc's memory  allocator
#   define STRDUP       strdup                      // allocates a new key (when it is a string)
#   define FREE(k)      free(k)                     // free a key (when it is a string)
#   ifdef _HOST_
#       define PRINT(...)   fprintf(stderr,__VA_ARGS__) 
#       define MALLOC_P(l)
#   else
#       define PRINT(...)   fprintf(0,__VA_ARGS__) 
#       define MALLOC_P(l)  malloc_print(l)
#   endif
#endif

/**
 * \brief   Iterates over possible slots for a given key using double hashing.
 *          Probes the hash table using double hashing to locate a slot for the key.
 *          It starts from the primary hash index and iterates through alternative 
 *          positions by incrementing the probe try (`try`).
 * \param   ht    Pointer to the hash table.
 * \param   key   The key whose slot positions are being probed.
 * \param   try   Declared internally; tracks the number of probing tries.
 * \param   h     Declared internally; stores the computed slot index.
 * \note    Ensures that all possible slots are explored in case of collisions.
 *          The variables `ht` and `key` must be declared before calling this macro.
 *          The variables `try` and `h` are internally declared and usable in the loop.
 */
#define FOREACH_PROBE(ht, key, try, h) \
    for (int try = 0, size = (ht)->size, h = hash (ht, (key), try); \
         try < size; \
         h = hash (ht, key, ++try))

//--------------------------------------------------------------------------------------------------
// opaque hash table structure
// This definition is private for this file only. All accesses are done through API functions only. 
//--------------------------------------------------------------------------------------------------

typedef struct hto_slot_s {          
    void *key;                                      ///< key is always a string
    void *val;                                      ///< value is a generic pointer (could be int)
} hto_slot_t;

struct hto_s {
    unsigned type:1;                                ///< ket=y type:  0=string  1=void*
    unsigned size:30;                               ///< Total number of slots in the hash table
    unsigned empty;                                 ///< Nb of completely empty slots (never used)
    unsigned freed;                                 ///< Nb of free slots (occupied but now deleted)
    struct hto_slot_s bucket[];                     ///< Array of `size` slots containing key-value
};

//--------------------------------------------------------------------------------------------------
// internal private functions
//--------------------------------------------------------------------------------------------------

/**
 * \brief   Finds the largest prime number less than or equal to the given number.
 * \param   n   A positive integer greater than 1.
 * \return  The largest prime number ≤ n, or -1 if no valid prime is found.
 */
static int largest_prime (unsigned n)
{
    for (int i, p; n > 1; n--) {                    // Try all numbers from n down to 2
        for (p=1, i=2; i*i <= n && p; p = n % i++); // Check divisibility up to sqrt (n)
        if (p) return n;                            // If p is still 1, n is prime, return it
    }
    return -1;                                      // Should never happen unless n = 1
}

/**
 * \brief   Computes the slot index for a given key using double hashing.
 *          This method ensures better key distribution and minimizes clustering.
 * \param   ht   The hash table in which the key is being searched.
 * \param   key  The key to be hashed.
 * \param   try  The probing try number (from 0 to ht->size - 1).
 * \return  The slot index where the key could be located.
 * \note    if h1 is the primary hashing value and h2 the secondary hashing value
 *          the function returns (h1 + try * h2) % ht->size
 *          h2 and size must be coprime to ensure that all slots can be probed,
 *          preventing poor key distribution and clustering. If they are not coprime,
 *          some slots may never be visited, reducing the efficiency of the hash table.
 */
static unsigned hash (const hto_t *ht, void *key, int try)
{
    unsigned long h1, h2;
    if (ht->type == 0) {                        // key is a string
        h1 = 5381;                              // DJB2: Daniel J. Bernstein version 2 (tinydns)
        h2 = 0;                                 // SDBM: Static DataBase Manager (awk)
        int c;
         
        while ((c = *(char*)key++)) {
            h1 = ((h1<<5) + h1) + c;            // DJB2: hash * 33 + c
            h2 = c + (h2<<6) + (h2<<16) - h2;   // SDBM
        }
    } else {                                    // key is a void *
        h1 = h2 = (unsigned long)key;
        h1 = h1 * 2654435761ull;
        h2 = (h2 << 6) + (h2 << 16);
    }
    h2 = (h2 % (ht->size - 1)) + 1;             // Ensure h2 is coprime with size, [1..size-1] 
    return (h1 + try * h2) % ht->size;          // Compute the probing position for the key
}

/**
 * \brief   Compare 2 keys 
 * \param   ht   The hash table in which the keys k1 and k2 are
 * \param   k1   The first key
 * \param   k2   The second key
 * \return  0 if equal ; negative number is k1 < k2 ; positive number if k1 > k2
 */
static unsigned keycmp (const hto_t *ht, void *k1, void *k2)
{
    return (ht->type) ? (unsigned long)k1 - (unsigned long)k2
                      : strcmp (k1, k2);  
}

/**
 * \brief   make a duplicate copy of a key
 * \param   ht   The hash table in which the key k is
 * \param   k    The key to duplicate
 * \return  the duplicated key
 */
static void * keydup (const hto_t *ht, void *k)
{
    return (ht->type) ? k : STRDUP(k);  
}

//--------------------------------------------------------------------------------------------------
// public API functions
//--------------------------------------------------------------------------------------------------

hto_t * hto_create (unsigned nb, int type)          // type: 0 key "char *" ; 1 if key "void *"
{
    nb = largest_prime (nb);                        // the number of entries must be a prime
    size_t s = sizeof(hto_t)+nb*sizeof(hto_slot_t); // header part + bucket part
    if (s > 4096) return NULL;                      // FIXME should be PAGE_SIZE for this version
    hto_t *ht = MALLOC(s);                          // allocate the hash table
    if (ht) {                                       // if alloc is a success    
        for (int i = 0; i < nb; i++) {              // for each slot
            ht->bucket[i].key = NULL;               // erase all
            ht->bucket[i].val = NULL;               // erase all
        }
        ht->size = ht->empty = nb;                  // table is empty
        ht->freed = 0;                              // thus no freed yet
        ht->type = type & 1;                        // even keys are char*; odd  keys are void*
    }
    return ht;                                      // return a real pointer or NULL
}

void hto_destroy( hto_t *ht, void (*freekeyfn)(void *), void (*freevalfn)(void *))
{
    for (unsigned s = ht->size, h = 0; h < s; h++) {// for each slot
        void *key = ht->bucket[h].key;              // get the current key
        if (key != NULL && key != KEYFREED) {       // if the slot is used
            if (freekeyfn)                          // if there is something to do
                freekeyfn (ht->bucket[h].key);      // free the key
            if (freevalfn)                          // if there is something to do
                freevalfn (ht->bucket[h].val);      // free the val     
        }
    }
    FREE (ht);
}

void * hto_get (hto_t *ht, void *key)               // see comment in htopen.h
{
    struct hto_slot_s * slot = NULL;                // will be the best slot
    FOREACH_PROBE(ht, key, try, h) {                // For each possible slot for this key
        void *current_key = ht->bucket[h].key;      // get the key at position h
        if (current_key == KEYFREED) {              // if first freed slot, will be the best slot
            if (!slot) slot = &(ht->bucket[h]);     // it is the fist freed slot found
            continue;                               // next try
        } 
        if (current_key == NULL) {                  // key not found
             return NULL;
        }
        if (keycmp (ht, key, current_key)==0) {     // we found the key
             void * val = ht->bucket[h].val;        // that is the found value
             if (slot) {                            // if a better slot was found 
                 ht->bucket[h].key = KEYFREED;      // we free the last found
                 slot->key = current_key;           // we move the current_key
                 slot->val = val;                   // then attach the current val
             }                                     
             return val;                            // at last return the found value
        }
    }
    return NULL;                                    // key not found
}

int hto_set (hto_t *ht, void *key, void *val)       // see comment in htopen.h
{
    struct hto_slot_s * slot = NULL;                // will be the best slot
    int try_forthisslot = 0;
    if (key == KEYFREED) return -2;                 // wrong key, KEYFREED is forbidden
    FOREACH_PROBE(ht, key, try, h) {                // For each possible slot for this key
        void * current_key = ht->bucket[h].key;     // get the key at position h
        if (current_key == KEYFREED) {              // if first freed slot, will be the best slot
            if (!slot) {                            // it is the fist freed slot found
                slot = &(ht->bucket[h]);            // remember the slot
                try_forthisslot = try;              // the number of try for this slot
            }
            continue;                               // next try
        } 
        if (current_key == NULL) {                  // key not found
            if (!slot) {                            // if we have not found a freed slot
                slot = &(ht->bucket[h]);            // the chosen slot is the current one
                ht->empty--;                        // new slot, thus one less empty slot
            } else {
                try = try_forthisslot;              // redefine the try counter
                ht->freed--;                        // reused slot, thus one less freed slot
            }
            slot->key = keydup (ht, key);           // we need to allocate the new key
            slot->val = val;                        // then attach the new val 
            return try;                             // return the number of try
        }
        if (keycmp (ht, key, current_key)==0) {     // we found the key
            if (slot) {                             // we found a better slot
                ht->bucket[h].key = KEYFREED;       // we free the last found
                slot->key = current_key;            // we move the current_key
                slot->val = val;                    // then attach the new val
            } else {
                ht->bucket[h].val = val;            // attach the new val
            }
            return  try;                            // at last return the number of try
        }
    }
    
    // the key is not in the hash table
    if (slot) {                                     // there is at least one freed slot
        ht->freed--;                                // reuse the slot 
        slot->key = keydup (ht, key);               // we need to allocate the new key
        slot->val = val;                            // then attach the new val 
        return try_forthisslot;                     // return the number of try for this slot
    }
    return -ENOSPC;                                 // -ENOSPC means hash table is full
}

int hto_set_grow (hto_t **pht, void *key, void *val, int maxtry)// see comment in htopen.h
{
    int try;
    for (try = hto_set (*pht, key, val);            // try to set an new item
        (try == -1) || (try > maxtry);              // if ht full or too much try
         hto_rehash (pht, 200),                     // rehash after growing the table
         try = hto_set (*pht, key, val));           // try again
    return try;
}

void * hto_del (hto_t *ht, void *key)               // see comment in htopen.h
{
    FOREACH_PROBE(ht, key, try, h) {                // For each possible slot for this key
        void *current_key = ht->bucket[h].key;      // get the key at position h
        if (current_key == KEYFREED) continue;      // if it it FREED, try to find the next position
        if (current_key == NULL) return NULL;       // key not found, thus return NULL
        if (keycmp (ht, key, current_key)==0) {     // key found
            void * old_val = ht->bucket[h].val;     // if the user want to free the old val
            FREE(ht->bucket[h].key);                // we must free the key, if it exists
            ht->bucket[h].key = KEYFREED;           // the slot is now FREED
            ht->bucket[h].val = NULL;               // just to clean the slot
            ht->freed++;                            // one more freed slot
            return old_val;                         // if the user would want to free the val   
        }
    }
    return NULL;                                    // key not found
}

void hto_foreach (hto_t *ht, hto_callback_t fn, void * data) // see comment in htopen.h
{
    for (unsigned s = ht->size, h = 0; h < s; h++) {// for each slot
        void *key = ht->bucket[h].key;              // get the current key
        if (key != NULL && key != KEYFREED) {       // if the slot is used
            void *val = ht->bucket[h].val;          // get the current val
            fn (ht, h, key, val, data);             // call the callback function 
        }
    }
}

/**
 * \brief   Rehashes a hash table with a given size factor.
 *          This function resizes the hash table by a given percentage and reinserts
 *          all existing elements to optimize performance and reduce clustering.
 *          If allocation fails, the original table remains unchanged.
 * \param   pht     Pointer to the hash table pointer (modified if rehash succeeds).
 * \param   percent Resize factor (100 = same size, 200 = double, 50 = half).
 * \return  NULL if rehashing fails, otherwise returns the new table pointer.
 */
hto_t *hto_rehash (hto_t **pht, unsigned percent)   // see comment in htopen.h
{
    if (!pht || !*pht || percent == 0) return NULL; // Invalid input
    hto_t *ht = *pht;                               // Dereference to get the actual table

    unsigned new_size = (ht->size * percent) / 100; // Compute new size
    if (new_size < 2) return NULL;                  // Avoid too small tables
    new_size = largest_prime (new_size);             // Find next prime for better hash distribution

    hto_t *new_ht = hto_create (new_size, ht->type); // Allocate new hash table
    if (!new_ht) return NULL;                       // Allocation failure, return NULL

    for (unsigned i = 0; i < ht->size; i++) {       // Reinsert all valid items manually
        void *key = ht->bucket[i].key;
        if (key && key != KEYFREED) {               // Only reinsert valid keys
            void *val = ht->bucket[i].val;
            hto_set (new_ht, key, val);
        }
    }

    FREE(ht);                                       // Free old table
    *pht = new_ht;                                  // Update caller’s table pointer
    return new_ht;                                  // Return new table pointer
}

//--------------------------------------------------------------------------------------------------
// Function to find out how to use the hash table
//--------------------------------------------------------------------------------------------------

/**
 * \brief   Callback function to analyze key collisions in the hash table.
 *          This function is used as a callback for `hto_foreach ()` in `hto_stat ()` (see below).
 *          It searches for a given key in the hash table and records the number of probes 
 *          (`try` attempts) required to locate it. This helps evaluate the efficiency 
 *          of the hashing function and the collision resolution strategy.
 * \param   ht    Pointer to the hash table where the key is being searched.
 * \param   pos   The slot index currently being examined (not used in this function).
 * \param   key   The key stored in the slot.
 * \param   val   The value associated with the key (not used in this function).
 * \param   data  Pointer to an array of `ht->size` elements tracking the distribution 
 *                of keys based on the number of probes required.
 * \note    This function does not modify the hash table; it only collects and prints
 *          collision statistics to assess the performance of the hashing mechanism.
 */
static inline void hto_collision (hto_t * ht, unsigned pos, void *key, void *val, void *data)
{
    unsigned int *tries = (unsigned int *)data;     // maximum number of collision
    FOREACH_PROBE(ht, key, try, h) {                // For each possible slot for this key
        void *current_key = ht->bucket[h].key;      // Get the current key
        if (current_key == KEYFREED) continue;      // Skip freed slots
        if (keycmp (ht, key, current_key)) continue;// Skip if not the searched key
        tries[try]++;                               // key found within "try" attempts
        return;                                     // End search
    }
}

void hto_stat (hto_t *ht)                     // see comment in htopen.h
{
    unsigned *tries = MALLOC(ht->size*sizeof(unsigned));
    unsigned nbkeys = ht->size - ht->freed - ht->empty;
    unsigned nbkeys_here = 0;
    
    if (tries == NULL) {
        PRINT("Impossible to allocate tries table\n");
        return;
    }
    for (int i=0; i < ht->size; tries[i++]=0);
    PRINT("nb keys + filled : %d --> %d%%\n", nbkeys, nbkeys*100/ht->size);     
    PRINT("hash table slots : %d\n", ht->size);     
    PRINT("hash table freed : %d\n", ht->freed);     
    PRINT("hash table empty : %d\n", ht->empty);     
    hto_foreach (ht, hto_collision, (void *)tries);
    for (int i=0; i < ht->size; i++) {
        nbkeys_here += tries[i];
        if (tries[i]) {
            PRINT("tries[%d]\t= %d (%d%% --> %d%%)\n", 
                i, tries[i], tries[i]*100/nbkeys, nbkeys_here*100/nbkeys);
        }
    }
    FREE (tries);
}

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
