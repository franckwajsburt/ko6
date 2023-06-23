/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-03
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     kernel/kmemory.c
  \author   Franck Wajsburt
  \brief    kernel allocators and user memory management  

  This file contains 2 allocators :
  * Slab allocator dedicated to the use of the kernel for all its objects
  * User stack allocator, only used for user threads stack (allocated by thread_create_kernel())

  Slab allocator

    * Allocate objects (aligned on a cache line) into slabs.
    * A slab is a segment (aligned on a page) that contains a single type of object.
    * In general, for large objects (> 1/8 page = 512 bytes), larger slabs (2 pages or 4 pages)
      are used to reduce external fragmentation.
    * The size of allocated objects is one page maximum.

    Structure list_t Slab[]
      * For allocation, the main structure is the Slab[] table.
      * Slab[] is indexed by the size of the object in cache line units (CacheLineSize).
      * The same size free objects are chained together with the ?list? API.
      * Slab[i] is thus the root of objects which are *CacheLineSize bytes
      * Slab[0] is for objects of a whole page, thus Slab[0] is the root of  the whole pages list

    void memory_init (void)
      * All cells of Slab[] table are initialized empty (with list_init()) but slab[0]
      * Slab[0] is initialized with all free pages.

    void * kmalloc (size_t size)
      * For these following cases, if there is a problem, it is a panic situation. That's the end.
        * the desired size is greater than one page
        * the desired size is one page, but there is no more free page availabble
      * The usual case is
        * calculate nbline that is the minimum number of cache lines containing the requested size
        * get the first object of Slab[nbline] list (with list_getfirst()) and return it
      * If there is no more free objects in Slab[nbline]
        * call kmalloc(PAGE_SIZE) itself to get a full page*
        * split the new page into as many free object of nbline size,
        * each piece is added to the Slab[nbline] list (with list_addlast())
        * then get the first object of Slab[nbline] list (with list_getfirst()) and return it
      * The return object is cleared (with 0) to reduce the risk of fealure in case of bad reuse

    void kfree (void * addr, size_t size)
      * For these following cases, if there is a problem, it is a panic situation. That's the end.
        * the addr is not inside the kernel heap
        * the size is too bif
      * The usual case is
        * calculate nbline that is the minimum number of cache lines containing the requested size
        * put the object back at the beginning of the list Slab[nbline]
        * if the size is exactly one page, return kfree
        * otherwise, decrement the number of objects allocated in the current slab/page
        * if this number is 0 then unlink all objects belonging to the current slab/page
        * then put the tile/page back in slab[0] (free pages)

\*------------------------------------------------------------------------------------------------*/

#include <klibc.h>                      

extern char __bss_end;                  // first char above bss section see kernel.ld (page aligned)
extern char __kdata_end;                // first char above the kernel data region (page aligned)
#define kmb (char*)&__bss_end           /* kernel memory begin */
#define kme (char*)&__kdata_end         /* kernel memory end */
static list_t FreeUserStack;            // free stack
static size_t CacheLineSize;            // max between 16 and the true cache line size
static size_t MaxLinePage;              // Max number of lines in a page = PAGE_SIZE/CacheLineSize
static list_t Slab[256];                // free lists, Slab[i]-> i*CacheLineSize, Slab[0]-> pages
static size_t Objects[256];             // Objects[i]= number of allocated objets of i*CacheLineSize
typedef struct page_s {                 // page usage description
    char slab;                          // Which slab does the page belong to? (0 is for page)
    char alloc;                         // number of allocated objects is slab
    char is_dirty:1;                    // 1 bit in 1 byte
} page_t;
page_t DummyPage;                       // this variable is to never have Page undefined
page_t *Page = &DummyPage;              // page descriptor table Page[O] is for page kmb, and so on
size_t NbPages;                         // maximum number of pages

#define NBLINE(n) (((n)+CacheLineSize-1)/CacheLineSize)     // minimal number of lines for n bytes

void memory_init (void)
{
    CacheLineSize = CEIL(cachelinesize(),16);               // true line size, but expand to 16 min
    NbPages = (kme-kmb)/PAGE_SIZE;                          // maximum number of pages
    MaxLinePage = PAGE_SIZE / CacheLineSize;                // 256 when line is 16, 128 for 32, etc.

    PANIC_IF (NbPages*sizeof(page_t) > PAGE_SIZE,           // Not more than 2048 thus 8MB
        "kmalloc can't handled %d pages", NbPages);         // write a message then panic

    list_init (&FreeUserStack);                             // initialize the free user stack list

    for (int i = 0 ; i < MaxLinePage ; i++)                 // initialize each list is Slab table
        list_init (&Slab[i]);                               // Slab[i] -> i*cachelinesize objects
    for (char *p = kmb; p != kme; p += PAGE_SIZE)           // initialize the page (slab) list
        list_addlast (&Slab[0], (list_t *)p);               // pointed by Slab[0]
    Page = kmalloc (PAGE_SIZE);                             // allocate the Page decritor table
    kmalloc_test(0,PAGE_SIZE/8);
}

//--------------------------------------------------------------------------------------------------

void * kmalloc (size_t size)
{
    PANIC_IF (size > PAGE_SIZE,                             // kmalloc is for small object
        "%d is too big, more than a single page", size);    // write a message then panic
    PANIC_IF ((size==PAGE_SIZE) && list_isempty (&Slab[0]), // free page are listed in Slab[0]
        "No more kernel data space");                       // write a message then panic

    size_t nbline = NBLINE(size);                           // required lines for size
    size = nbline * CacheLineSize;                          // actual size asked

    if ((size!=PAGE_SIZE) && list_isempty (&Slab[nbline])) {// if no more object in the slab
        char *page = kmalloc (PAGE_SIZE);                   // ask for a free page (i.e. slab)
        size_t npage = (size_t)(page - (char *)kmb)>>12;    // relative page number from kmb
        Page[npage].alloc = 0;                              // reset the allocated counter
        for (char *p=page; p+size<=page+PAGE_SIZE; p+=size) // cut the slab into objects
            list_addlast (&Slab[nbline], (list_t *)p);      // and chain them together
    }
    void * res = list_getfirst (&Slab[nbline%MaxLinePage]); // res is the first object in list
    size_t npage = (size_t)(res - (void *)kmb)>>12;         // relative page number from kmb
    Objects[nbline%MaxLinePage]++;                          // increment the number of objects
    Page[npage].slab = nbline%MaxLinePage;                  // this page is used of slab of nbline
    Page[npage].alloc++;                                    // one more times

    wzero (res, size);                                      // clear allocated memory
    return res;                                             // finally returns res
}

void kfree (void * obj, size_t size)
{
    size_t nbline= NBLINE(size) % MaxLinePage;              // which slab to use
    size_t npage = (size_t)(obj - (void *)kmb)>>12;         // relative page number from kmb

    PANIC_IF ((nbline>255)||(npage >= NbPages),             // too big or outside of the region
        "\ncan't free object not allocated by kmalloc()");  // write a message then panic

    list_addfirst (&Slab[nbline], (list_t *)obj);           // add it to the right free list
    Objects[nbline]--;                                      // decr the number of obj of size nbline
    if (size == PAGE_SIZE) return;
    if (--Page[npage].alloc==0) {                           // splitted page and no more object left
        list_t *page = (list_t *)((size_t)obj & ~0xFFF);    // address of the page containing obj
        list_foreach (&Slab[nbline], item) {                // browse all item in free list
            size_t np_item = (size_t)((char*)item-kmb)>>12; // page number Page[] table
            if (np_item == npage) {                         // if current item is in the page
                list_unlink (item);                         // unlink it
            }
        }
        Page[npage].slab = 0;                               // since the page is empty, thus slab 0
        list_addfirst (&Slab[0], (list_t *)page);           // add the free page in slab[O]
        Objects[0]--;                                       // decr the number of pages used
    }
}

//--------------------------------------------------------------------------------------------------

void kmalloc_print (void)
{
    size_t cr = 0, pr = 1;
    kprintf ("\nOpen Slab[] : Object Size ; Free Objects ; Allocated Objects\n");
    for (size_t nbline=0 ; nbline<MaxLinePage ; nbline++) { // for all slabs
        size_t sz = (nbline)?nbline*CacheLineSize:4096;     // size really allocated
        size_t nf = list_nbobj (&Slab[nbline]);             // number of free obj of size nline
        size_t na = Objects[nbline];                        // number of allocated obj of size nline
        if (nf+na) {                                        // if there is something to print
            kprintf ("|s %d\t f %d\t a %d", sz, nf, na);    // print data
            kprintf ((++cr%3)?"\t":"\t|\n");                // adds a \n all three print
            pr=0;
        }
    }
    kprintf ((cr+pr)?"\n":"");                              // adds a \n if not just added it
    cr = 0; pr = 1;
    kprintf ("\nUsed Memory Pages  : Slab Type (nb cache lines) ; Allocated Objects\n");
    for (size_t p = 0; p < NbPages; p++) {                  // for all pages
        size_t ps = Page[p].slab;                           // for what slab[] it is used
        size_t pa = Page[p].alloc;                          // how many alloc objects there are in
        if (pa) {                                           // if the page contain allocated object
            kprintf ("|p %d\t s %d\t a %d",p,ps,pa);        // print data
            kprintf ((++cr%3)?"\t":"\t|\n");                // adds a \n all three print
            pr=0;

        }
    }
    kprintf ((cr+pr)?"\n":"");                              // adds a \n if not just added it
}

static list_t KMallocTest[256];         // free lists, Slab[i]-> i*CacheLineSize, Slab[0]-> pages
void kmalloc_test (size_t turn, size_t size)
{
    list_t *obj;                                            // object allocated or freed
    if (turn == 0) return;                                  // if no turn forgive now
    kprintf ("kmalloc test %s turn %d size max %d\n",       // a small message
             __func__,turn,size);
    for (int i = 0 ; i < MaxLinePage ; i++)                 // nitialize KMallocTest
        list_init (&KMallocTest[i]);                        // KMallocTest[i] -> i*cachelinesize
    while (turn--) {                                        // the number of turn is configurable
        int sz = 1+(rand()+CacheLineSize/2) % size;         // choose a size
        size_t nbline = NBLINE(sz);                         // required lines for size
        nbline %= MaxLinePage;                              // if sz==PAGE_SIZE use Slab[0]
        if (rand()%2) {                                     // alloc or free
            obj = kmalloc (sz);                             // allocate a new object
            list_addlast (&KMallocTest[nbline], obj);       // add it in allocated object
        } else {                                            // else
            obj = list_getfirst (&KMallocTest[nbline]);     // try to get a previously allocated obj
            if (obj) kfree (obj, nbline*CacheLineSize);     // on success, free it
        }
    }
    kmalloc_print();                                        // Slab status after alloc/free series
    for (int i = 0 ; i < MaxLinePage ; i++) {               // for all list
        list_foreach (&KMallocTest[i], item) {              // browse allocated objects
            list_unlink (item);                             // if found it, unlinked it
            kfree (item,i*CacheLineSize);                   // then free it
        }
    }
    kmalloc_print();                                        // Slab status after total clean od test
}

//--------------------------------------------------------------------------------------------------

void * sbrk (int increment)
{
    errno = SUCCESS;
    int * a = _user_mem.uheap_end + increment/sizeof(int);  // sizeof() because uheap_end is int*
    a = (int *) FLOOR (a, CacheLineSize);                   // addr 'a' could be the new uheap_end
    if ((a<_user_mem.uheap_beg)||(a>_user_mem.ustack_end)){ // if it is outside the heap zone
        errno = ENOMEM;
        return (void *)-1;                                  // -1 on failure
    }
    return a;                                               // else return a;
}

//--------------------------------------------------------------------------------------------------

int * malloc_ustack (void)
{
    int * top;                                              // top will be the new stack pointer
    int * end = (int *)list_getlast (&FreeUserStack);       // get last free stack (biggest addr)
    if (end == NULL) {                                      // if there is no more free stack
        top = _user_mem.ustack_end;                         // try to get one
        end = top - USTACK_SIZE/sizeof(int);                // and compute the end of the stack
        PANIC_IF (end < _user_mem.uheap_end,                // if the stack end is in the heap
            "no more space for user stack!\n");             // it is impossible to solve that
        _user_mem.ustack_end = end;                         // expand the stacks' region
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
 * \param   newx    is the new item to insert
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

    if (end ==_user_mem.ustack_end) {                       // if it is the lowest stack
        _user_mem.ustack_end += USTACK_SIZE/sizeof(int);    // shrink the stacks' region
        list_foreach (&FreeUserStack, stack) {              // foreach free stack
            if ((int *)stack != _user_mem.ustack_end)       // if it isn't the end of stack region 
                break;                                      // then stop trying to shrink
            end = (int *)list_getfirst (&FreeUserStack);    // extract the stack
            end += USTACK_SIZE/sizeof(int);                 // new end of stacks'region
            _user_mem.ustack_end = end;                     // save this new end
        }
    } else                                                  // else the freed stack isn't at the end
        list_addsort (&FreeUserStack,(list_t*)end,cmp_addr);// add it in free list in order
}

void print_ustack (void)
{
    kprintf ("---------------\nNumber of stacks : %d\n", 
            ((char *)(_user_mem.ustack_beg) - 
             (char *)(_user_mem.ustack_end) )/USTACK_SIZE);
    kprintf ("_user_mem.ustack_beg : %p\n", _user_mem.ustack_beg);
    kprintf ("_user_mem.ustack_end : %p\n", _user_mem.ustack_end);
    kprintf ("_user_mem.uheap_beg  : %p\n", _user_mem.uheap_beg );
    kprintf ("_user_mem.uheap_end  : %p\n", _user_mem.uheap_end );
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
        int place = (unsigned)rand() % NBSTACK;
        if (stack[place]) 
            free_ustack (stack[place]);    
        stack[place] = malloc_ustack();
    }
    for (int place = 0; place < NBSTACK; place++)  
        if (stack[place]) 
            free_ustack (stack[place]);    
    print_ustack();
}
