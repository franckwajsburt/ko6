/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-02-17
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     common/ht_prob.c
  \author   Franck Wajsburt
  \brief    Efficient Hash Table with double hash probing (Open Addressing)
            Uses double hashing (`h(k) + i * h2(k) mod N`) for collision resolution,
            ensuring better key distribution and avoiding clustering.
            Also features on-the-fly rehashing during `set` and `get` to optimize key placement.

\*------------------------------------------------------------------------------------------------*/

#include <ht_prob.h>

extern int DEBUG;

#ifdef _KERNEL_                                     // if it is for the kernel
#   define MALLOC       kmalloc                     // allocates in the slab allocator
#   define STRDUP       kstrdup                     
#   define FREE(k)      kfree(k)
#   define PRINT(...)   kprintf(0, __VA_ARGS__) 
#else                                               // if it is for the user
#   define MALLOC       malloc                      // allocates in the libc's memory  allocator
#   define STRDUP       strdup
#   define FREE(k)      free(k)
#   define PRINT(...)   fprintf(stderr,__VA_ARGS__) 
#endif

#define FREED           ((char *)-1)                // key used when a slot is freed 

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
    for (int try = 0, size = (ht)->size, h = hash((key), try, ht); \
         try < size; \
         h = hash(key, ++try, ht))

//--------------------------------------------------------------------------------------------------
// opaque hash table structure
// This definition is private for this file only. All accesses are done through API functions only. 
//--------------------------------------------------------------------------------------------------

struct ht_slot_s {          
    char *key;                                      // key is always a string
    void *val;                                      // value is a generic pointer (could be an int)
};

struct ht_s {
    size_t size;                                    // Total number of slots in the hash table
    size_t empty;                                   // Nb of completely empty slots (never used)
    size_t freed;                                   // Nb of freed slots (occupied but now deleted)
    struct ht_slot_s bucket[];                      // Array of `size` slots containing key-value
};

//--------------------------------------------------------------------------------------------------
// internal private functions
//--------------------------------------------------------------------------------------------------

/**
 * \brief   Finds the largest prime number less than or equal to the given number.
 * \param   n   A positive integer greater than 1.
 * \return  The largest prime number ≤ n, or -1 if no valid prime is found.
 */
static int largest_prime (size_t n)
{
    for (int i, p; n > 1; n--) {                    // Try all numbers from n down to 2
        for (p=1, i=2; i*i <= n && p; p = n % i++); // Check divisibility up to sqrt(n)
        if (p) return n;                            // If p is still 1, n is prime, return it
    }
    return -1;                                      // Should never happen unless n = 1
}

/**
 * \brief   Computes the slot index for a given key using double hashing.
 *          This method ensures better key distribution and minimizes clustering.
 * \param   key  The key to be hashed.
 * \param   try  The probing try number (from 0 to ht->size - 1).
 * \param   ht   The hash table in which the key is being searched.
 * \return  The slot index where the key could be located.
 * \note    if h1 is the primary hashing value and h2 the secondary hashing value
 *          the function returns (h1 + try * h2) % ht->size
 *          h2 and size must be coprime to ensure that all slots can be probed,
 *          preventing poor key distribution and clustering. If they are not coprime,
 *          some slots may never be visited, reducing the efficiency of the hash table.
 */
static size_t hash(const char *key, int try, const ht_t *ht)
{
    unsigned long h1 = 5381;                        // DJB2: Daniel J. Bernstein version 2 (tinydns)
    unsigned long h2 = 0;                           // SDBM: Static DataBase Manager (awk)
    int c;

    while ((c = *key++)) {
        h1 = ((h1 << 5) + h1) + c;                  // DJB2: hash * 33 + c
        h2 = c + (h2 << 6) + (h2 << 16) - h2;       // SDBM
    }

    h2 = (h2 % (ht->size - 1)) + 1;                 // Ensure h2 is coprime with size, [1..size-1] 
    return (h1 + try * h2) % ht->size;              // Compute the probing position for the key
}

/**
 * \brief   Callback function to analyze key collisions in the hash table.
 *          This function is used as a callback for `ht_foreach()` in `ht_stat()` (see below).
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
static inline void ht_collision(ht_t * ht, size_t pos, const char *key, void *val, void *data)
{
    unsigned char *tries = (unsigned char *)data;   // maximum number of collision
    FOREACH_PROBE(ht, key, try, h) {                // For each possible slot for this key
        const char *current_key = ht->bucket[h].key;// Get the current key
        if (current_key == FREED) continue;         // Skip freed slots
        if (strcmp (key, current_key)) continue;    // Skip if not the searched key
        if (tries[try] != 255)                      // more than 255 try
            tries[try]++;
        return;                                     // End search
    }
}

//--------------------------------------------------------------------------------------------------
// public API functions
//--------------------------------------------------------------------------------------------------

ht_t * ht_create (size_t size)                      // see comment in ht_probe.h
{
    size = largest_prime(size);                     // size must be a prime
    ht_t *ht = MALLOC (                             // allocate the hash table
                  3 * sizeof(size_t) +              // header part
                  size * sizeof(struct ht_slot_s)); // bucket part
    if (ht) {                                       // if alloc is a success    
        for (int i = 0; i < size; i++) {            // for each slot
            ht->bucket[i].key = NULL;               // erase all
            ht->bucket[i].val = NULL;               // erase all
        }
        ht->size = ht->empty = size;                // table is empty
        ht->freed = 0;                              // thus no freed yet
    }
    return ht;                                      // return a real pointer or NULL
}

void * ht_get (ht_t *ht, const char *key)           // see comment in ht_probe.h
{
    struct ht_slot_s * slot = NULL;                 // will be the best slot
    FOREACH_PROBE(ht, key, try, h) {                // For each possible slot for this key
        char *current_key = ht->bucket[h].key;      // get the key at position h
        if (current_key == FREED) {                 // if first freed slot, will be the best slot
            if (!slot) slot = &(ht->bucket[h]);     // it is the fist freed slot found
            continue;                               // next try
        } 
        if (current_key == NULL) {                  // key not found
             return NULL;
        }
        if (strcmp (key, current_key)==0) {         // we found the key
             void * val = ht->bucket[h].val;        // that is the found value
             if (slot) {                            // if a better slot was found 
                 ht->bucket[h].key = FREED;         // we free the last found
                 slot->key = current_key;           // we move the current_key
                 slot->val = val;                   // then attach the current val
             }                                     
             return val;                            // at last return the found value
        }
    }
    return NULL;                                    // key not found
}

int ht_set (ht_t *ht, const char *key, void *val)// see comment in ht_probe.h
{
    struct ht_slot_s * slot = NULL;                 // will be the best slot
    int try_forthisslot = 0;
    FOREACH_PROBE(ht, key, try, h) {                // For each possible slot for this key
        char * current_key = ht->bucket[h].key;     // get the key at position h
        if (current_key == FREED) {                 // if first freed slot, will be the best slot
            if (!slot) {                            // it is the fist freed slot found
                slot = &(ht->bucket[h]);            // remember the slot
                try_forthisslot = try;               // the number of try for this slot
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
            slot->key = STRDUP (key);               // we need to allocate the new key
            slot->val = val;                        // then attach the new val 
            return try;                             // return the number of try
        }
        if (strcmp (key, current_key)==0) {         // we found the key
            if (slot) {                             // we found a better slot
                ht->bucket[h].key = FREED;          // we free the last found
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
        slot->key = STRDUP (key);                   // we need to allocate the new key
        slot->val = val;                            // then attach the new val 
        return try_forthisslot;                     // return the number of try for this slot
    }
    return -1;                                      // -1 means hash table is full
}

int ht_set_grow (ht_t **pht, const char *key, void *val, int maxtry)// see comment in ht_probe.h
{
    int try;
    while ((try = ht_set(*pht, key, val)) > maxtry){// try to set val 
        ht_rehash (pht, 200);                       // but automatically rehash if try > maxtry
    }
    return try;
}

void * ht_del (ht_t *ht, const char *key)           // see comment in ht_probe.h
{
    FOREACH_PROBE(ht, key, try, h) {                // For each possible slot for this key
        const void *current_key = ht->bucket[h].key;// get the key at position h
        if (current_key == FREED) continue;         // if it it FREED, try to find the next position
        if (current_key == NULL) return NULL;       // key not found, thus return NULL
        if (strcmp (key, current_key)==0) {         // key found
            void * old_val = ht->bucket[h].val;     // if the user want to free the old val
            FREE(ht->bucket[h].key);                // we must free the key, if it exists
            ht->bucket[h].key = FREED;              // the slot is now FREED
            ht->bucket[h].val = NULL;               // just to clean the slot
            ht->freed++;                            // one more freed slot
            return old_val;                         // if the user would want to free the val   
        }
    }
    return NULL;                                    // key not found
}

void ht_foreach(ht_t *ht, ht_callback_t callback, void * data) // see comment in ht_probe.h
{
    for (size_t s = ht->size, h = 0; h < s; h++) {  // for each slot
        char *key = ht->bucket[h].key;              // get the current key
        if (key != NULL && key != FREED) {          // if the slot is used
            void *val = ht->bucket[h].val;          // get the current val
            callback(ht, h, key, val, data);        // call the callback function 
        }
    }
}

void ht_stat(ht_t *ht)                              // see comment in ht_probe.h
{
    unsigned char *tries = MALLOC(ht->size);
    if (tries == NULL) {
        PRINT ("Impossible to allocate tries table\n");
        return;
    }
    for (int i=0; i < ht->size; tries[i++]=0);
    PRINT ("hash table slots : %zu\n", ht->size);     
    PRINT ("hash table freed : %zu\n", ht->freed);     
    PRINT ("hash table empty : %zu\n", ht->empty);     
    ht_foreach (ht, ht_collision, (void *)tries);
    for (int i=0; i < ht->size; i++)
        if (tries[i])
            PRINT ("tries[%d]\t= %d\n", i, tries[i]);
    FREE(tries);
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
ht_t *ht_rehash(ht_t **pht, size_t percent)         // see comment in ht_probe.h
{
    if (!pht || !*pht || percent == 0) return NULL; // Invalid input
    ht_t *ht = *pht;                                // Dereference to get the actual table

    size_t new_size = (ht->size * percent) / 100;   // Compute new size
    if (new_size < 2) return NULL;                  // Avoid too small tables
    new_size = largest_prime(new_size);             // Find next prime for better hash distribution

    ht_t *new_ht = ht_create(new_size);             // Allocate new hash table
    if (!new_ht) return NULL;                       // Allocation failure, return NULL

    for (size_t i = 0; i < ht->size; i++) {         // Reinsert all valid items manually
        const char *key = ht->bucket[i].key;
        if (key && key != FREED) {                  // Only reinsert valid keys
            void *val = ht->bucket[i].val;
            ht_set(new_ht, key, val);
        }
    }

    FREE(ht);                                       // Free old table
    *pht = new_ht;                                  // Update caller’s table pointer
    return new_ht;                                  // Return new table pointer
}
