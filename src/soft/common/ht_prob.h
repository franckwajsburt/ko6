/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-02-17
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     common/ht_prob.h
  \author   Franck Wajsburt
  \brief    Efficient Hash Table with double hash probing (Open Addressing)
            Uses double hashing (`h(k) + i * h2(k) mod N`) for collision resolution,
            ensuring better key distribution and avoiding clustering.
            Also features on-the-fly rehashing during `set` and `get` to optimize key placement.

\*------------------------------------------------------------------------------------------------*/

#define HT_MAXTRY   10

#ifdef _HOST_
#   include <stdlib.h>
#   include <stddef.h>
#   include <stdio.h>
#   include <string.h>
#elif defined  _KERNEL_
#   include <klibc.h>
#else
#   include <libc.h>
#endif


typedef struct ht_s ht_t;
/**
 * \brief   Opaque structure representing a hash table.
 *          This hash table uses open addressing with double hashing to resolve collisions.
 *          Each entry consists of a string key and a generic pointer value.
 */
typedef struct ht_s ht_t;

/**
 * \brief   Creates a new hash table with open addressing.
 *          The function initializes a hash table of the given size,
 *          ensuring that the table size is a prime number to optimize double hashing.
 *          All slots are initialized as empty.
 * \param   size The requested initial size of the hash table.
 * \return  A pointer to the newly allocated hash table, or NULL if allocation fails.
 * \note    The actual size of the table may be slightly larger than the requested size
 *          because it is adjusted to the nearest prime number for better hashing performance.
 */
ht_t * ht_create (size_t size);

/**
 * \brief   Retrieves the value associated with a given key in the hash table.
 *          This function searches for a key using double hashing to determine 
 *          the slot index. If the key is found, its associated value is returned.
 *          Additionally, if a freed slot was encountered during the search, 
 *          the key is relocated to that slot to improve future lookup performance.
 * \param   ht   Pointer to the hash table.
 * \param   key  The key to be searched for.
 * \return  A pointer to the value associated with the key, or NULL if the key is not found.
 * \note    If a freed slot is encountered before finding the key, the function moves
 *          the key to that slot. This helps reduce fragmentation and optimizes future lookups.
 *          If the key does not exist in the table, NULL is returned.
 */
void * ht_get (ht_t *ht, const char *key);

/**
 * \brief   Inserts a key-value pair into the hash table.
 *          This function inserts a new key or updates an existing key in the hash table.
 *          It uses double hashing to determine the slot index. If a freed slot is encountered 
 *          before finding an empty slot, the function prioritizes reusing that freed slot 
 *          to minimize fragmentation. If the key already exists, its value is updated.
 * \param   ht   Pointer to the hash table.
 * \param   key  The key to be inserted (will be duplicated internally).
 * \param   val  The value associated with the key.
 * \return  The number of probes required for insertion.
 *          If the table is full (no available slot), returns -1.
 * \note    If a freed slot is found during the search, the function moves the key to that slot
 *          to optimize key placement and improve future lookups, reducing fragmentation.
 *          Additionally, the function duplicates the key using `STRDUP()`, so the caller is 
 *          responsible for freeing the original key if needed.
 */
int ht_set(ht_t *ht, const char *key, void *val);

/**
 * \brief   Inserts a key-value pair into the hash table, automatically growing the table 
 *          if insertion exceeds a maximum number of probes.
 * \param   pht    Pointer to the hash table pointer. May be updated if resizing occurs.
 * \param   key    The key to be inserted (will be duplicated internally).
 * \param   val    The value associated with the key.
 * \param   maxtry Maximum number of probes allowed before attempting to grow the table.
 * \return  The number of probes required before insertion succeeds.
 *          Automatically grows the table if necessary. Returns -1 if resizing fails.
 */
int ht_set_grow(ht_t **pht, const char *key, void *val, int maxtry);

/**
 * \brief   Deletes a key from the hash table.
 *          This function searches for a key in the hash table and removes it if found.
 *          The key's memory is freed, and the slot is marked as `FREED` to indicate that 
 *          it was previously occupied. This allows future insertions to reuse freed slots.
 * \param   ht   Pointer to the hash table.
 * \param   key  The key to be deleted.
 * \return  A pointer to the value associated with the deleted key (so the caller can free it),
 *          or NULL if the key was not found.
 * \note    The key's memory is freed, but the caller is responsible for managing
 *          the memory of the returned value if necessary.
 *          The function does not shrink the hash table, it only marks slots as `FREED`. 
 *          This may lead to fragmentation if too many deletions occur without insertions.
 */
void * ht_del (ht_t *ht, const char *key);

/**
 * \brief   Function pointer type for callbacks used in `ht_foreach`.
 *          A callback function of this type is executed for each occupied slot in the hash table. 
 *          It receives the slot index, the key, the associated value and a user-defined data ptr
 * \param   ht    Pointer to the hash table.
 * \param   pos   The index of the slot in the hash table.
 * \param   key   The key stored in the slot.
 * \param   val   The value associated with the key.
 * \param   data  A user-defined pointer passed to provide context to the callback function.
 */
typedef void (*ht_callback_t)(ht_t *ht, size_t pos, const char *key, void *val, void *data);

/**
 * \brief   Iterates over all occupied slots in the hash table and applies a callback function.
 *          This function traverses all slots in the hash table and calls the given callback 
 *          function on each occupied slot (i.e., slots that contain a valid key). 
 *          The callback function follows the `ht_callback_t` signature (see above)
 * \param   ht        Pointer to the hash table.
 * \param   callback  Function of type `ht_callback_t` that will be called for each valid entry.
 * \param   data      A user-defined pointer passed to the callback function for additional context.
 *          The callback function is called only for valid entries (empty or freed slots skippe).
 */
void ht_foreach(ht_t *ht, ht_callback_t callback, void *data);

/**
 * \brief   Displays statistics about the hash table.
 *          This function prints the total number of slots, 
 *          the number of freed slots (previously occupied but now available), 
 *          and the number of empty slots (never occupied). 
 *          It also analyzes key distribution by detecting collisions 
 * \param   ht  Pointer to the hash table.
 */
void ht_stat (ht_t *ht);

/**
 * \brief   Rehashes a hash table with a given size factor.
 *          This function resizes the hash table by a given percentage and reinserts
 *          all existing elements to optimize performance and reduce clustering.
 *          If allocation fails, the original table remains unchanged.
 * \param   pht     Pointer to the hash table pointer (modified if rehash succeeds).
 * \param   percent Resize factor (100 = same size, 200 = double, 50 = half).
 * \return  NULL if rehashing fails, otherwise returns the new table pointer.
 */
ht_t *ht_rehash(ht_t **pht, size_t percent);
