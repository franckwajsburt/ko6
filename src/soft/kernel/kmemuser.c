/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-13
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     kernel/kmemory.c
  \author   Franck Wajsburt
  \brief    user memory allocators from the kernel point of view

  * user stack allocator, only used for user threads stack (allocated by thread_create_kernel())

\*------------------------------------------------------------------------------------------------*/

#include <kernel/klibc.h>

// Variables and Define hardware dependant ---------------------------------------------------------

static size_t CacheLineSize;            // max between 16 and the true cache line size

//--------------------------------------------------------------------------------------------------

static list_t FreeUserStack;            // free stack

void kmemuser_init() 
{
    CacheLineSize = CEIL(cachelinesize(),16);               // true line size, but expand to 16 min
    list_init (&FreeUserStack);                             // initialize the free user stack list
}

int * malloc_ustack (void)
{
    int * top;                                              // top will be the new stack pointer
    int * end = (int *)list_getlast (&FreeUserStack);       // get last free stack (biggest addr)
    if (end == NULL) {                                      // if there is no more free stack
        top = __usermem.ustack_end;                         // try to get one
        end = top - USTACK_SIZE/sizeof(int);                // and compute the end of the stack
        PANIC_IF (end < __usermem.uheap_end,                // if the stack end is in the heap
            "no more space for user stack!\n");             // it is impossible to solve that
        __usermem.ustack_end = end;                         // expand the stacks' region
    } else {
        top = end + USTACK_SIZE/sizeof(int);                // compute stack's top from stack's end
    }
    top--;                                                  // get a word to put MAGIC
    *top = *end = MAGIC_STACK;                              // to be able to check free
    return top;                                             // finally return the top
}

/**
 * \brief   compare two item address, used by list_addsort in free_ustack()
 * \param   curr    is the current item in the list
 * \param   new     is the new item to insert
 * \return  a positive if current > new
 */

static int cmp_addr (list_t * curr, list_t * new) {
    return (int)(curr - new);
}

void free_ustack (int * top)
{
    int * end = 1 + top - USTACK_SIZE/sizeof(int);          // last int of ustack (+1 because MAGIC)
    PANIC_IF (*top != MAGIC_STACK, "Corrupted top Stack");  // if no magic number then panic
    PANIC_IF (*end != MAGIC_STACK, "Corrupted end Stack");  // if no magic number then panic

    if (end ==__usermem.ustack_end) {                       // if it is the lowest stack
        __usermem.ustack_end += USTACK_SIZE/sizeof(int);    // shrink the stacks' region
        list_foreach (&FreeUserStack, stack) {              // foreach free stack
            if ((int *)stack != __usermem.ustack_end)       // if it isn't the end of stack region
                break;                                      // then stop trying to shrink
            end = (int *)list_getfirst (&FreeUserStack);    // extract the stack
            end += USTACK_SIZE/sizeof(int);                 // new end of stacks'region
            __usermem.ustack_end = end;                     // save this new end
        }
    } else                                                  // else the freed stack isn't at the end
        list_addsort (&FreeUserStack,(list_t*)end,cmp_addr);// add it in free list in order
}

void print_ustack (void)
{
    kprintf ("---------------\nNumber of stacks : %d\n",
            ((char *)(__usermem.ustack_beg) -
             (char *)(__usermem.ustack_end) )/USTACK_SIZE);
    kprintf ("__usermem.ustack_beg : %p\n", __usermem.ustack_beg);
    kprintf ("__usermem.ustack_end : %p\n", __usermem.ustack_end);
    kprintf ("__usermem.uheap_beg  : %p\n", __usermem.uheap_beg );
    kprintf ("__usermem.uheap_end  : %p\n", __usermem.uheap_end );
    kprintf ("----\nFree stacks : \n");
    list_foreach (&FreeUserStack, item) {
        kprintf ("Address %p\n", item);
    }
}

void test_ustack (size_t turn)
{
#   define NBSTACK    10
    int * stack [NBSTACK] = {NULL};
    while (turn--) {
        int place = (unsigned)krand() % NBSTACK;
        if (stack[place])
            free_ustack (stack[place]);
        stack[place] = malloc_ustack();
    }
    for (int place = 0; place < NBSTACK; place++)
        if (stack[place])
            free_ustack (stack[place]);
    print_ustack();
}

//--------------------------------------------------------------------------------------------------

void * sbrk (int increment)
{
    errno = SUCCESS;
    int * a = __usermem.uheap_end + increment/sizeof(int);  // sizeof() because uheap_end is int*
    a = (int *) FLOOR (a, CacheLineSize);                   // addr 'a' could be the new uheap_end
    if ((a<__usermem.uheap_beg)||(a>__usermem.ustack_end)){ // if it is outside the heap zone
        errno = ENOMEM;
        return (void *)-1;                                  // -1 on failure
    }
    return a;                                               // else return a;
}

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
