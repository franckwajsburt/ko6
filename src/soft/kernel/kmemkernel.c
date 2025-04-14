/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-13
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     kernel/kmemkernel.c
  \author   Franck Wajsburt
  \brief    kernel allocators 

  This file contains the kernel memory management
  * The physical page management
  * Slab allocator dedicated to the use of the kernel for all its objects

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
        * calculate lines that is the minimum number of cache lines containing the requested size
        * lines is actually the type of slab, and thus the slab number
        * get the first object of Slab[lines] list (with list_getfirst()) and return it
      * If there is no more free objects in Slab[lines]
        * call kmalloc(PAGE_SIZE) itself to get a full page*
        * split the new page into as many free object of nbline size,
        * each piece is added to the Slab[lines] list (with list_addlast())
        * then get the first object of Slab[lines] list (with list_getfirst()) and return it
      * The return object is cleared (with 0) to reduce the risk of fealure in case of bad reuse

    void kfree (void * addr)
      * For these following cases, if there is a problem, it is a panic situation. That's the end.
        * the addr is not inside the kernel heap
      * The usual case is
        * retreave the page number to discover the slab type thanks to the Page[] structure
        * put the object back at the beginning of the list Slab[lines]
        * if the size is exactly one page, return kfree
        * otherwise, decrement the number of objects allocated in the current slab/page
        * if this number is 0 then unlink all objects belonging to the current slab/page
        * then put the tile/page back in slab[0] (free pages)

\*------------------------------------------------------------------------------------------------*/

#include <kernel/klibc.h>

// Variables and Define hardware dependant ---------------------------------------------------------

extern int __kbss_origin;               // first int of bss section (defined in ldscript kernel.ld)
extern int __kbss_end;                  // 1st char above kbss section see kernel.ld (page aligned)
extern int __kdata_end;                 // 1st char above the kernel data region (page aligned)
#define kmb (char*)&__kbss_end          // kernel memory begin
#define kme (char*)&__kdata_end         // kernel memory end 
static size_t CacheLineSize;            // max between 16 and the true cache line size
static size_t MaxLineSlab;              // Max number of lines in a page = PAGE_SIZE/CacheLineSize
static size_t NbPages;                  // Total number of pages in kernel memory

#define DATARAMSIZE (256<<20)           // Maximum kernel memory 256MB (must be > NbPages<<12)
#define NBLINE(n) (((n)+CacheLineSize-1)/CacheLineSize) // minimal number of lines for n bytes
#define SEGFAULT(addr) (((char *)(addr) < kmb)||((char *)(addr) >= kme))
#define PAGEINDEX(page) (size_t)(((char *)(page)-(char *)kmb)>>12;
#define PAGE(page) ((((char*)(page) - kmb)>>12)%NbPages)

// Variables for kernel memory allocator -----------------------------------------------------------

enum {
    PAGE_SLAB,
    PAGE_BLOCK
};

typedef union {                         // page usage description
    unsigned long long raw;             // 64 bits 
    struct {
        unsigned type:1;                // type of page SLAB
        unsigned reserved:15;           // not used yet
        unsigned lines:8;               // object size in number of cache lines 
        unsigned nbused:8;              // number of used objects is this slab
    } slab;
    struct {
        unsigned type:1;                // type of page BLOCK
        unsigned dirty:1;               // dirty  --> must be written on disk
        unsigned locked:1;              // locked --> once read, must remains on memory
        unsigned valid:1;               // valid  --> data can be read or write
        unsigned bdev:4;                // to have several disks 
        unsigned refcount:8;            // the same block can be open several times
        unsigned reserved:16;           // not used yet
        unsigned lba;                   // Logic Block Address (where the page is on disk)
    } block;
} page_t;

static page_t Page[DATARAMSIZE>>12];    // DATARAMSIZE / 4kB (size = ((256<<20)>>12)<<3 = 512 kB

static list_t Slab[256];                // free lists, Slab[i]-> i*CacheLineSize, Slab[0]-> pages
static size_t ObjectsThisSize[256];     // ObjectsThisSize[i]= allocated objets of i*CacheLineSize


//--------------------------------------------------------------------------------------------------
// Page descriptor accessor functions
//--------------------------------------------------------------------------------------------------

void page_set_valid (void *page)    { Page[PAGE(page)].block.valid  = 1; }
void page_set_locked (void *page)   { Page[PAGE(page)].block.locked = 1; }
void page_set_dirty (void *page)    { Page[PAGE(page)].block.dirty  = 1; }

void page_clear_valid (void *page)  { Page[PAGE(page)].block.valid  = 0; }
void page_clear_locked (void *page) { Page[PAGE(page)].block.locked = 0; }
void page_clear_dirty (void *page)  { Page[PAGE(page)].block.dirty  = 0; }

int  page_is_valid (void *page)     { return Page[PAGE(page)].block.valid;  }
int  page_is_locked (void *page)    { return Page[PAGE(page)].block.locked; }
int  page_is_dirty (void *page)     { return Page[PAGE(page)].block.dirty;  }

int  page_get_refcount (void *page) { return Page[PAGE(page)].block.refcount; }

void page_inc_refcount (void *page) {
    int refcount = Page[PAGE(page)].block.refcount;
    PANIC_IF (refcount == 255, "Too much page references: %p\n", page);
    Page[PAGE(page)].block.refcount = refcount+1;
}

void page_dec_refcount (void *page) {
    int refcount = Page[PAGE(page)].block.refcount;
    PANIC_IF (refcount == 0, "Page reference is already 0: %p\n", page);
    Page[PAGE(page)].block.refcount = refcount-1;
}

void page_set_lba(void *page, unsigned bdev, unsigned lba) {
    Page[PAGE(page)].block.bdev = bdev; 
    Page[PAGE(page)].block.lba  = lba; 
}

void page_get_lba(void *page, unsigned *bdev, unsigned *lba) {
    if (bdev) *bdev = Page[PAGE(page)].block.bdev ; 
    if (lba)  *lba  = Page[PAGE(page)].block.lba  ; 
}

//--------------------------------------------------------------------------------------------------
// Slab allocator
//--------------------------------------------------------------------------------------------------

void kmemkernel_init (void)
{
    // put kbss sections to zero. kbss contains uninitialised global variables of the kernel
    for (int *a = &__kbss_origin; a != &__kbss_end; *a++ = 0);

    CacheLineSize = CEIL(cachelinesize(),16);               // true line size, but expand to 16 min
    NbPages = (kme-kmb)/PAGE_SIZE;                          // maximum number of pages
    MaxLineSlab = PAGE_SIZE / CacheLineSize;                // 256 when line is 16, 128 for 32, etc.

    for (int i = 0 ; i < MaxLineSlab ; i++)                 // initialize each list is Slab table
        list_init (&Slab[i]);                               // Slab[i] -> i*cachelinesize objects
    for (char *p = kmb; p != kme; p += PAGE_SIZE)           // initialize the page (slab) list
        list_addlast (&Slab[0], (list_t *)p);               // pointed by Slab[0]
}

//--------------------------------------------------------------------------------------------------

void * kmalloc (size_t size)
{
    PANIC_IF (size > PAGE_SIZE,                             // kmalloc is for small object
        "%d is too big, more than a single page", size);    // write a message then panic
    PANIC_IF ((size==PAGE_SIZE) && list_isempty (&Slab[0]), // free page are listed in Slab[0]
        "No more kernel data space");                       // write a message then panic

    size_t lines = NBLINE(size);                            // required lines for size
    size = lines * CacheLineSize;                           // actual size asked
    
    if ((size!=PAGE_SIZE) && list_isempty (&Slab[lines])) { // if no more object in the slab
        char *page = kmalloc (PAGE_SIZE);                   // ask for a free page (i.e. slab)
        Page[PAGE(page)].slab.nbused = 0;                   // reset the allocated counter
        for (char *p=page; p+size<=page+PAGE_SIZE; p+=size) // cut the slab into objects
            list_addlast (&Slab[lines], (list_t *)p);       // and chain them together
    }
    void * res = list_getfirst (&Slab[lines%MaxLineSlab]);  // res is the first object in list
    ObjectsThisSize [lines % MaxLineSlab]++;                // increment the number of objects
    Page[PAGE(res)].slab.lines = lines % MaxLineSlab;       // this page is used as a slab of nbline
    Page[PAGE(res)].slab.nbused++;                          // one more times

    wzero (res, size);                                      // clear allocated memory
    return res;                                             // finally returns res
}

void *kcalloc(size_t n, size_t size)
{
    return kmalloc (n * size);                              // allocate (kmalloc clears the zone)
}

void kfree (void * obj)
{
    PANIC_IF (SEGFAULT(obj),                                // outside of the page region
        "\ncan't free object not allocated by kmalloc()");  // write a message then panic
    unsigned npage = PAGE(obj);
    size_t lines = Page[npage].slab.lines;                  // which slab to use
    list_addfirst (&Slab[lines], (list_t *)obj);            // add it to the right free list
    ObjectsThisSize[lines]--;                               // decr the number of obj of size lines
    if (lines == 0) return;
    if (--Page[npage].slab.nbused==0) {                     // splitted page and no more object left
        list_t *page = (list_t *)((size_t)obj & ~0xFFF);    // address of the page containing obj
        list_foreach (&Slab[lines], item) {                 // browse all item in free list
            if (PAGE(item) == npage) {                      // if current item is in the page
                list_unlink (item);                         // unlink it
            }
        }
        Page[npage].slab.lines = 0;                         // since the page is empty, thus lines 0
        list_addfirst (&Slab[0], (list_t *)page);           // add the free page in slab[O]
        ObjectsThisSize[0]--;                               // decr the number of pages used
    }
}

char * kstrdup (const char * str) 
{
    PANIC_IF (str==NULL,"kstrdup called with NULL pointer");// Avoid NULL input 
    size_t len = strlen(str) + 1;                           // Include null terminator
    char *copy = (char *) kmalloc(len);                     // Allocate memory 
    PANIC_IF (copy==NULL,"kstrdup: out of memory");         // Check allocation failure
    memcpy(copy, str, len);                                 // Copy the string, including '\0'
    return copy;                                            // Return the allocated copy
}

//--------------------------------------------------------------------------------------------------

void kmalloc_stat (void)
{
    size_t cr = 0, pr = 1;
    kprintf ("\nTotal Kernel Memory Size : %d pages = %d.%d MBytes\n", \
            NbPages, NbPages/256, NbPages%256);
    kprintf ("\nObjects distribution in all slabs \n");
    kprintf ("\n(s) Object Size ; (f) Free Objects ; (a) Allocated Objects\n");
    for (size_t lines=0 ; lines<MaxLineSlab ; lines++) {    // for all slabs
        size_t sz = (lines) ? lines*CacheLineSize : 4096;   // size really allocated
        size_t nf = list_nbobj (&Slab[lines]);              // number of free obj of size nline
        size_t na = ObjectsThisSize[lines];                 // number of allocated obj of size nline
        if (nf+na) {                                        // if there is something to print
            kprintf ("|s %d\tf %d\ta %d", sz, nf, na);      // print data
            kprintf ((++cr%3)?"\t":"\t|\n");                // adds a \n all three print
            pr=0;
        }
    }
    kprintf ((cr+pr)?"\n":"");                              // adds a \n if not just added it
    cr = 0; pr = 1;
    kprintf ("Memory Pages Usage\n");
    kprintf ("\n(p) Page Number ; (s) Object Size ; (a) Allocated Objects\n");
    for (size_t p = 0; p < NbPages; p++) {                  // for all pages
        size_t ps = Page[p].slab.lines * CacheLineSize;     // for what slab[] it is used
        size_t pa = Page[p].slab.nbused;                    // how many alloc objects there are in
        if (pa) {                                           // if the page contain allocated object
            kprintf ("|p %d\ts %d\ta %d",p,ps,pa);          // print data
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
    for (int i = 0 ; i < MaxLineSlab ; i++)                 // nitialize KMallocTest
        list_init (&KMallocTest[i]);                        // KMallocTest[i] -> i*cachelinesize
    while (turn--) {                                        // the number of turn is configurable
        int sz = 1+(krand()+CacheLineSize/2) % size;        // choose a size
        size_t lines = NBLINE(sz);                          // required lines for size
        lines %= MaxLineSlab;                               // if sz==PAGE_SIZE use Slab[0]
        if (krand()%2) {                                    // alloc or free
            obj = kmalloc (sz);                             // allocate a new object
            list_addlast (&KMallocTest[lines], obj);        // add it in allocated object
        } else {                                            // else
            obj = list_getfirst (&KMallocTest[lines]);      // try to get a previously allocated obj
            if (obj) kfree (obj);                           // on success, free it
        }
    }
    kmalloc_stat ();                                        // Slab status after alloc/free series
    for (int i = 0 ; i < MaxLineSlab ; i++) {               // for all list
        list_foreach (&KMallocTest[i], item) {              // browse allocated objects
            list_unlink (item);                             // if found it, unlinked it
            kfree (item);                                   // then free it
        }
    }
    kmalloc_stat ();                                        // Slab status after total clean od test
}

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
