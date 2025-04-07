/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-05
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \licence    https://opensource.org/licenses/MIT

  \file     common/radix.c
  \author   Franck Wajsburt
  \brief    A radix tree one-dimensional table : void * radix[0x100000000]

    API
        radix <-- radix_create()        : create the table, i.e. the radix tree
        radix_get(radix, index)         : get the value from radix[index]
        radix_set(radix, index, val)    : set radix[index] with val
        radix_destroy(radix)            : free all the radix, but not the stored values

    index is 32 bits, split into 4 levels of 8 bits: index=L0.L1.L2.L3 (where L0 is MSB, L3 is LSB)
    It is thus a 4-level radix tree with 8-bit slices.
    Each node of the radix tree has 256 slots for each level.
    radix [index] : node[L0] --> node[L1] --> node[L2] --> node[L0]
    values stored node[L0] are void* pointers, which are in most cases pointers to 4kB-pages
    There are a 1 single node for le level 0.
    There are a 256 nodes for le level 1.
    There are a 256 x 256 nodes for le level 2.
    There are a 256 x 256 x 256 nodes for le level 3.

    root_l0 --> node[L0]

    There is an optmimisation when the index is small,
    only the necessary tree depth is allocated, saving both time and memory.
    root_l1 = root_l0[0]
    root_l2 = root_l0[0] --> node[0]
    root_l3 = root_l0[0] --> node[0] --> node[0]

                   ┌─┐
         ┌─────────┤ │├── L0(index)
         │       ┌─┤1│
         │       │ │0│
         │ ----- │ └┬┘◄───────────── root_l0   if (index >= 0x1000000)
         ▼       ▼ ┌▼┐
         ┌─────────┤ │├── L1(index)
         │       ┌─┤1│
         │       │ │0│
         │ ----- │ └┬┘◄───────────── root_l1   if (index < 0x1000000)
         ▼       ▼ ┌▼┐
         ┌─────────┤ │├── L2(index)
         │       ┌─┤1│
         │       │ │0│
         │ ----- │ └┬┘◄───────────── root_l2   if (index < 0x1000)
         ▼       ▼ ┌▼┐
         ┌─────────┤ │├── L3(index)
         │       ┌─┤1│
         │       │ │0│
         │ ----- │ └┬┘◄───────────── root_l3   if (index < 0x100)
        ┌▼┐     ┌▼┐┌▼┐
        │ │pages│ ││ │    void*
        └─┘     └─┘└─┘

    The radix tree increases with the addition of elements,
    but it is not reduced when the elements are deleted.
    However, it can be freed in its entirety with radix_destroy.

\*------------------------------------------------------------------------------------------------*/

#define V(v,...) fprintf( stderr, "%s : %lx %s", #v, (unsigned long)v,__VA_ARGS__);

#include <radix.h>
#ifdef _KERNEL_                                     // if it is for the kernel
#   include <kernel/klibc.h>
#   define CALLOC       kcalloc                     // allocates in the slab allocator
#   define FREE(k)      kfree(k)                    // free a key (when it is a string)
#   define PRINT(...)   kprintf(__VA_ARGS__) 
#else                                               // if it is for the user
#   define CALLOC       calloc                      // allocates in the libc's memory  allocator
#   define FREE(k)      free(k)                     // free a key (when it is a string)
#   ifdef _HOST_
#     define PRINT(...) fprintf(stderr,__VA_ARGS__) 
#   else
#     define PRINT(...) fprintf(0,__VA_ARGS__) 
#   endif
#endif

#define RADIX_SLOTS  256                                    ///< nb slots by node

#define L0(i)  (((i) >> 24) & 0xFF)                         ///< most significant byte
#define L1(i)  (((i) >> 16) & 0xFF)
#define L2(i)  (((i) >> 8) & 0xFF)
#define L3(i)  ((i) & 0xFF)                                 ///< least significant byte
#define INDEX(i0,i1,i2,i3) (((i0)<<24)|((i1)<<16)|((i2)<<8)|(i3)) ///< rebuilt index from slices

typedef struct radix_node_s {                               ///< radix tree node
    void *slots[RADIX_SLOTS];                               ///< if 256 --> node=1024 bytes
} radix_node_t;

struct radix_s {
    radix_node_t *root_l0;                                  ///< radix root if L0 != 0
    radix_node_t *root_l1;                                  ///< radix root if L0 == 0
    radix_node_t *root_l2;                                  ///< radix root if L0 == L1 == 0
    radix_node_t *root_l3;                                  ///< radix root if L0 == L1 == L2 == 0
};

radix_t *radix_create(void) {                               ///< create the main structure
    return calloc(1, sizeof(radix_t));                      //   empty
}

static radix_node_t *new_node(void) {                       ///< create an empty node
    return calloc(1, sizeof(radix_node_t));
}

void *radix_get(const radix_t *rx, unsigned index) {        ///< get rx[index]
    radix_node_t *l0, *l1, *l2, *l3;
    if (index < 0x100) {                                    // if small index < 256
        if (!(l3 = rx->root_l3)) return NULL;               // maybe node absent then NULL
        return l3->slots[ L3(index) ];                      // if node exists then read it
    }
    if (index < 0x10000) {                                  // if L0 == L1 == 0
        if (!(l2 = rx->root_l2)) return NULL;               // maybe node not yet allocated
        if (!(l3 = l2->slots[ L2(index) ])) return NULL;    // get the next level if allocated
        return l3->slots[ L3(index) ];                      // if node exists then read it
    }
    if (index < 0x1000000) {                                // if L0 == 0
        if (!(l1 = rx->root_l1)) return NULL;               // node not yet allocated
        if (!(l2 = l1->slots[ L1(index) ])) return NULL;    // next level if allocated
        if (!(l3 = l2->slots[ L2(index) ])) return NULL;    // next level if allocated
        return l3->slots[ L3(index) ];                      // if node exists then read it
    }
    /// general case whenever all levels have to be path, if index > 0x1000000
    if (!(l0 = rx->root_l0)) return NULL;
    if (!(l1 = l0->slots[ L0(index) ])) return NULL;
    if (!(l2 = l1->slots[ L1(index) ])) return NULL;
    if (!(l3 = l2->slots[ L2(index) ])) return NULL;
    return l3->slots[ L3(index) ];
}

int radix_set(radix_t *rx, unsigned index, void *val)       ///< rx[index] <-- val
{
    radix_node_t *l0, *l1, *l2, *l3;
    
    if (index < 0x100) {                                    // index < 256
        if (!(l3 = rx->root_l3)) {                          // if root_l3 not yet allocated
            if (!(l3 = new_node())) return -1;              // try to allocate the node
            rx->root_l3 = l3;
        }
        l3->slots[ L3(index) ] = val;                       // if success, set the value
        return 0;                                           // success
    }
    if (index < 0x10000) {                                  // index < 256 x 256
        if (!(l2 = rx->root_l2)) {                          // if root_l2 not yet allocated
            if (!(l2 = new_node())) return -1;              // try to allocate the node
            l2->slots[ 0 ] = rx->root_l3;                   // if there is already a root_l3;
            rx->root_l2 = l2;                               // new root_l2
        }
        if (!(l3 = l2->slots[ L2(index) ])) {               // find next level & test if allocated
            if (!(l3 = new_node())) return -1;              // if not try to allocate it
            l2->slots[ L2(index) ] = l3;                    // connect the mode l3
            if (L2(index) == 0) rx->root_l3 = l3;           // update the root_l3 if needed
        }
        l3->slots[ L3(index) ] = val;                       // if success, set the value
        return 0;                                           // success
    }
    if (index < 0x1000000) {                                // index < 256 x 256 x 256
        if (!(l1 = rx->root_l1)) {
            if (!(l1 = new_node())) return -1;
            if (!rx->root_l2 && rx->root_l3) {
                if (!(rx->root_l2 = new_node())) return -1;
                rx->root_l2->slots[0] = rx->root_l3;
            }    
            l1->slots[ 0 ] = rx->root_l2;
            rx->root_l1 = l1;
        }
        if (!(l2 = l1->slots[ L1(index) ])) {
            if (!(l2 = new_node())) return -1;
            l1->slots[ L1(index) ] = l2;
            if (L1(index) == 0) rx->root_l2 = l2;
        }
        if (!(l3 = l2->slots[ L2(index) ])) {
            if (!(l3 = new_node())) return -1;
            l2->slots[ L2(index) ] = l3;
            if (L2(index) == 0) rx->root_l3 = l3;
        }
        l3->slots[ L3(index) ] = val;
        return 0;
    }                                                       // general case

    if (!(l0 = rx->root_l0)) {
        if (!(l0 = new_node())) return -1;
        if (!rx->root_l2 && rx->root_l3) {
            if (!(rx->root_l2 = new_node())) return -1;
            rx->root_l2->slots[0] = rx->root_l3;
        }     
        if (!rx->root_l1 && rx->root_l2) {
            if (!(rx->root_l1 = new_node())) return -1;
            rx->root_l1->slots[0] = rx->root_l2;
        }     
        l0->slots[ 0 ] = rx->root_l1;
        rx->root_l0 = l0;
    }
    if (!(l1 = l0->slots[ L0(index) ])) {
        if (!(l1 = new_node())) return -1;
        l0->slots[ L0(index) ] = l1;
        if (L0(index) == 0) rx->root_l1 = l1;
    }
    if (!(l2 = l1->slots[ L1(index) ])) {
        if (!(l2 = new_node())) return -1;
        l1->slots[ L1(index) ] = l2;
        if (L1(index) == 0) rx->root_l2 = l2;
    }
    if (!(l3 = l2->slots[ L2(index) ])) {
        if (!(l3 = new_node())) return -1;
        l2->slots[ L2(index) ] = l3;
        if (L2(index) == 0) rx->root_l3 = l3;
    }
    l3->slots[ L3(index) ] = val;
    return 0;
}

void radix_destroy(radix_t *rx)                             ///< destroy the entire radix
{
    radix_node_t *l0, *l1, *l2, *l3;
    int i0, i1, i2;
    if (!rx) return;                                        // radix must exist

    if ((l0 = rx->root_l0)) {                               // there is a L0 level
        for (i0 = 0; i0 < RADIX_SLOTS; i0++) {              // scan all L1 nodes
            if (!(l1 = l0->slots[i0])) continue;            // L1 not allocate? give up & continue
            for (i1 = 0; i1 < RADIX_SLOTS; i1++) {          // if L1 allocated, scan all L2 nodes
               if (!(l2 = l1->slots[i1])) continue;         // L2 not allocate? give up & continue
               for (i2 = 0; i2 < RADIX_SLOTS; i2++)         // if L2 allocated, scan all L3 nodes
                   if ((l3 = l2->slots[i2])) free(l3);      // if L3 allocated, free that L3
               free(l2);                                    // scan done, free that L2
            }
            free(l1);                                       // scan done, free that L2
        }
        free(l0);                                           // scan done, free the root level
    }
    else if ((l1 = rx->root_l1)) {                          // L0 not exists but maybe L1 w. L0==0
        for (i1 = 0; i1 < RADIX_SLOTS; i1++) {              // if L1 allocated, scan all L2 nodes
           if (!(l2 = l1->slots[i1])) continue;             // L2 not allocate? give up & continue
           for (i2 = 0; i2 < RADIX_SLOTS; i2++)             // if L2 allocated, scan all L3 nodes
               if ((l3 = l2->slots[i2])) free(l3);            // if L3 allocated, free that L3
           free(l2);                                        // scan done, free that L2
        }
        free(l1);                                           // scan done, free the root level
    }
    else if ((l2 = rx->root_l2)) {                          // L0/L1 not exist but maybe L2 
        for (i2 = 0; i2 < RADIX_SLOTS; i2++)                // if L2 allocated scann all L3 nodes
           if ((l3 = l2->slots[i2])) free(l3);                // if L3 allocated, free that L3
        free(l2);                                           // scan done free the root level
    }
    free(rx);                                               // at last free a radix structure itself
}

void radix_foreach(const radix_t *rx, radix_callback_t fn, void *data)   ///< scan all elements
{
    radix_node_t *l0, *l1, *l2, *l3;    
    unsigned i0, i1, i2, i3;
    void *val;
    if (!rx || !fn) return;

    if ((l0 = rx->root_l0)) {                               // if L0 exists
        for (i0 = 0; i0 < RADIX_SLOTS; i0++) {              // scan all L1 nodes
            if (!(l1 = l0->slots[i0])) continue;            // if L1 node is empty, goto next L1
            for (i1 = 0; i1 < RADIX_SLOTS; i1++) {          // scan all L2 nodes
                if (!(l2 = l1->slots[i1])) continue;        // if L2 node is empty, goto next L2
                for (i2 = 0; i2 < RADIX_SLOTS; i2++) {      // scan all L3 nodes
                    if (!(l3 = l2->slots[i2])) continue;    // if L3 node is empty, goto next L3
                    for (i3 = 0; i3 < RADIX_SLOTS; i3++) {  // scan all values
                        if ((val = l3->slots[i3])) {        // if value is not NULL
                            fn(rx,INDEX(i0,i1,i2,i3),val,data);// callback
    }   }   }   }   }   } 
    else if ((l1 = rx->root_l1)) {
        for (i1 = 0; i1 < RADIX_SLOTS; i1++) {
            if (!(l2 = l1->slots[i1])) continue;
            for (i2 = 0; i2 < RADIX_SLOTS; i2++) {
                if (!(l3 = l2->slots[i2])) continue;
                for (i3 = 0; i3 < RADIX_SLOTS; i3++) {
                    if ((val = l3->slots[i3])) {
                        fn(rx,INDEX(0,i1,i2,i3), val, data);
    }   }   }   }   } 
    else if ((l2 = rx->root_l2)) {
        for (i2 = 0; i2 < RADIX_SLOTS; i2++) {
            if (!(l3 = l2->slots[i2])) continue;
            for (i3 = 0; i3 < RADIX_SLOTS; i3++) {
                if ((val = l3->slots[i3])) {
                    fn(rx,INDEX(0,0,i2,i3), val, data);
    }   }   }   } 
    else if ((l3 = rx->root_l3)) {
        for (i3 = 0; i3 < RADIX_SLOTS; i3++) {
            if ((val = l3->slots[i3])) {
                fn(rx,i3, val, data);
    }   }   }
}

/**
 * \brief   cleanup a radix tree
 * \param   rx    radix tree
 * \param   pnode pointer to node pointer 
 * \param   level from the top (level 0) to the leaf (level 3)
 * \param   index tracks the full path in base-256 to determine when the index is 0.
 * \return  Recursively frees empty nodes in the radix tree.
 *           1) free all allocated node whose all slots are NULL
 *           2) set to NULL the parent pointer if the child node is freed
 *              Also clears root_l1/l2/l3 if their respective subtrees become empty.
 */
static void cleanup_node(radix_t *rx, radix_node_t **pnode, int level, unsigned index) 
{
    radix_node_t *node = *pnode;
    int empty = 1;                                          // hyp. the node is empty

    for (int i = 0; i < RADIX_SLOTS; ++i) {                 // for all slot at that level
        if (node->slots[i] == NULL) continue;               // the slot is NULL, see next
        if (level == 3) return;                             // slot != 0 && level 3, node not empty 
        cleanup_node(rx, (radix_node_t **)&node->slots[i],  // cleanup each slot 
                    level + 1, (index<<8) | i);             // next level, compute index
        if (node->slots[i]) empty = 0;                      // test is the sub-tree is empty 
    }

    if (empty) {                                            // if all slots of the node are NULL
        free(node);                                         // free le node
        *pnode = NULL;                                      // set the parent pointer to NULL
        if (index == 0) {                                   // index of the intermediate root
                 if (level == 1) rx->root_l1 = NULL;        // erase intermediate roots
            else if (level == 2) rx->root_l2 = NULL;
            else if (level == 3) rx->root_l3 = NULL;
        }
    }
}

void radix_cleanup(radix_t *rx) 
{
    if (!rx) return;
         if (rx->root_l0) cleanup_node(rx, &rx->root_l0, 0, 0); // if root is at leve 0
    else if (rx->root_l1) cleanup_node(rx, &rx->root_l1, 1, 0); // if root is at leve 1
    else if (rx->root_l2) cleanup_node(rx, &rx->root_l2, 2, 0); // if root is at leve 2
    else if (rx->root_l3) cleanup_node(rx, &rx->root_l3, 3, 0); // if root is at leve 3
}

void radix_stat(const radix_t *rx)
{
    radix_node_t *l0, *l1, *l2, *l3;
    unsigned i0, i1, i2, i3;
    int values = 0;
    int nodes[4] = { [0 ... 3 ] = 0 };
    int maxlevel = 0;

    if (!rx) return;

    if ((l0 = rx->root_l0)) {
        maxlevel = 4;
        nodes[0] = 1;
        for (i0 = 0; i0 < RADIX_SLOTS; i0++) {
            if (!(l1 = l0->slots[i0])) continue;
            nodes[1]++;
            for (i1 = 0; i1 < RADIX_SLOTS; i1++) {
                if (!(l2 = l1->slots[i1])) continue;
                nodes[2]++;
                for (i2 = 0; i2 < RADIX_SLOTS; i2++) {
                    if (!(l3 = l2->slots[i2])) continue;
                    nodes[3]++;
                    for (i3 = 0; i3 < RADIX_SLOTS; i3++) {
                        if ((l3->slots[i3])) {
                            values++;
    }   }   }   }   }   }
    else if ((l1 = rx->root_l1)) {
        maxlevel = 3;
        nodes[1] = 1;
        for (i1 = 0; i1 < RADIX_SLOTS; i1++) {
            if (!(l2 = l1->slots[i1])) continue;
            nodes[2]++;
            for (i2 = 0; i2 < RADIX_SLOTS; i2++) {
                if (!(l3 = l2->slots[i2])) continue;
                nodes[3]++;
                for (i3 = 0; i3 < RADIX_SLOTS; i3++) {
                    if ((l3->slots[i3])) {
                        values++;
    }   }   }   }   }
    else if ((l2 = rx->root_l2)) {
        maxlevel = 2;
        nodes[2] = 1;
        for (i2 = 0; i2 < RADIX_SLOTS; i2++) {
            if (!(l3 = l2->slots[i2])) continue;
            nodes[3]++;
            for (i3 = 0; i3 < RADIX_SLOTS; i3++) {
                if ((l3->slots[i3])) {
                    values++;
    }   }   }   }
    else if ((l3 = rx->root_l3)) {
        maxlevel = 1;
        nodes[3] = 1;
        for (i3 = 0; i3 < RADIX_SLOTS; i3++) {
            if ((l3->slots[i3])) {
                values++;
    }   }   }
    PRINT ("nb values : %d\n", values);
    for (int i = 0 ; i < maxlevel ; i++)
        PRINT ("level %d   : %d\n", i, nodes[i]);
}

//--------------------------------------------------------------------------------------------------
// Debug function
//--------------------------------------------------------------------------------------------------

void radix_export_dot(const radix_t *rx, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return;

    fprintf(f, "digraph radix {\n");
    fprintf(f, "  node [shape=record];\n");

    // compteur d'ID pour nommer chaque noeud
    unsigned next_id = 0;

    void dump_node(const radix_node_t *node, unsigned level, unsigned id, unsigned index) {
        fprintf(f, "  node%u [label=\"{", id);
        for (int i = 0; i < RADIX_SLOTS; ++i) {
            if (node->slots[i])
                fprintf(f, "<f%u> %02X %p |", i, i, &(node->slots[i]));
        }
        fprintf(f, "}\"];\n");

        for (int i = 0; i < RADIX_SLOTS; ++i) {
            if (!node->slots[i]) continue;
            unsigned child_id = ++next_id;
            fprintf(f, "  node%u:f%u -> node%u;\n", id, i, child_id);
            if (level < 3)
                dump_node((radix_node_t *)node->slots[i], level + 1, child_id, (index << 8) | i);
            else
                fprintf(f, "  node%u [label=\"leaf %p\"];\n", child_id, node->slots[i]);
        }
    }

    if (rx->root_l0) dump_node(rx->root_l0, 0, next_id++, 0);
    else if (rx->root_l1) dump_node(rx->root_l1, 1, next_id++, 0);
    else if (rx->root_l2) dump_node(rx->root_l2, 2, next_id++, 0);
    else if (rx->root_l3) dump_node(rx->root_l3, 3, next_id++, 0);

    fprintf(f, "}\n");
    fclose(f);
}

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
