/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date        2025-02-24
  | / /(     )/ _ \     \copyright   2021 Sorbonne University
  |_\_\ x___x \___/                  https://opensource.org/licenses/MIT

  \file     hal/almo1/kfs.c
  \author   Franck Wajsburt
  \author   Angie Bikou
  \brief    kfs-lite ko6 file system lite

  FIXME     use type to forbid read/write/unlink
  TODO      clarify kfs usage in kfs_split/kfs_build and ko6

\*------------------------------------------------------------------------------------------------*/

#include <kfs.h>

//--------------------------------------------------------------------------------------------------
// a few generic declarations
//--------------------------------------------------------------------------------------------------

#ifndef NULL
#define NULL (void *)0
#endif
typedef unsigned int    u32_t;
typedef unsigned short  u16_t;
typedef unsigned char   u8_t;
typedef signed int      i32_t;
typedef signed short    i16_t;
typedef signed char     i8_t;

//--------------------------------------------------------------------------------------------------
// static structure definition of kfs
//--------------------------------------------------------------------------------------------------

#define KFS_MAX_NAME    28      /* dentry name length */

typedef struct kfs_dentry_s {   ///< 32 bytes long
    u8_t root;                  ///< index of the root dentry, 0 if it is /
    u8_t next;                  ///< index of one of the dentries at the same level
    u8_t leaf;                  ///< index of one of the leaf dentries (0 if no leaf)
    u8_t inode;                 ///< index of the associated inode
    char name[KFS_MAX_NAME];    ///< max size of a file or folder name
} kfs_dentry_t;

typedef struct kfs_inode_s {    ///< 32 bytes long
    u32_t type:2;               ///< KFS_FILE, KFS_DIR, KFS_PIPE, KFS_SLINK
    u32_t mode:6;               ///< rwxrwx for owner and others
    u32_t size:24;              ///< file size is 16mib max
    u16_t count:2;              ///< nb of inode references
    u16_t owner:2 ;             ///< owner : kernel, user1, user2, user3
    u16_t mtime:8;              ///< relative modification/creation time
    u16_t unused:4;             ///< we will see
    u16_t page[12];             ///< first 12 pages of a file (12*4kib=48kb)
    u16_t fmap;                 ///< page above the first 12 pages
} kfs_inode_t;

typedef u16_t kfs_fmap_t[16];   ///< fmap as inode extension, maps until 16 pages or 16 other fmaps

#define KFS_MAX_DENTRY  ((KFS_NPG_DENTRY<<12)/sizeof(kfs_dentry_t)) /* number of dentries */
#define KFS_MAX_INODE   ((KFS_NPG_INODE <<12)/sizeof(kfs_inode_t))  /* number of inodes */
#define KFS_MAX_FMAP    ((KFS_NPG_FMAP  <<12)/sizeof(kfs_fmap_t))   /* number of file maps */

typedef struct kfs_mbr_s {              ///< 1<<12 bytes long
    u32_t code[127];                    ///< bootloader loader
    u32_t magic;                        ///< 0xD15C4C06 seems like DISC4KO6
    u8_t  padding[(1<<12)-512];         ///< to complete the page
} kfs_mbr_t;

typedef struct kfs_sblock_s {           ///< size: KFS_NPG_SBLOCK<<12 bytes
    u32_t max_dentry;                   ///< total dentries number
    u32_t cur_dentry;                   ///< last dentry allocated/freed, all before it are occupied
    u8_t  bmp_dentry[KFS_MAX_DENTRY/8]; ///< bitmap of dentries
    u32_t max_inode;                    ///< total inodes number
    u32_t cur_inode;                    ///< last inode allocated/freed, all before it are occupied
    u8_t  bmp_inode[KFS_MAX_INODE/8];   ///< bitmap of inodes
    u32_t max_fmap;                     ///< total fmaps number
    u32_t cur_fmap;                     ///< last fmap allocated/freed, all before it are occupied
    u8_t  bmp_fmap[KFS_MAX_FMAP/8];     ///< bitmap of fmaps
    u32_t max_page;                     ///< total pages number
    u32_t cur_page;                     ///< last page allocated/freed, all before it are occupied
    u8_t  bmp_page[KFS_NPG_DISK/8];     ///< bitmap of pages
    u8_t  padding[(KFS_NPG_SBLOCK<<12)  ///< add space to occupy exactly the KFS_NPG_SBLOCK pages
                 - 8*sizeof(u32_t)      ///< minus all cur and max fields
                 - KFS_MAX_DENTRY/8     ///< minus all bitmaps spaces
                 - KFS_MAX_INODE/8
                 - KFS_MAX_FMAP/8
                 - KFS_NPG_DISK/8];
} kfs_sblock_t;

typedef u32_t kfs_page_t[1024];         ///< atomic data segment exchanged with the disk

//--------------------------------------------------------------------------------------------------
// global data, all static, only directly accessible inside this file (outside, we need accessors)
//--------------------------------------------------------------------------------------------------

#ifdef _HOST_
static kfs_mbr_t  KfsMbr;                           ///< Master Boot Record aligned on first page
static kfs_page_t KfsVbr[KFS_NPG_VBR];              ///< Mbr and Vbr are in memory only on Linux
#endif

#define NPGU (KFS_NPG_BOOT+KFS_NPG_META)            /* number of pages used by BOOT and META */
static kfs_sblock_t KfsSblock = {                   ///< sblock gives data on kfs partition
    .max_dentry = KFS_MAX_DENTRY,                   ///< initialize all max items of bipmaps
    .max_inode  = KFS_MAX_INODE,
    .max_fmap   = KFS_MAX_FMAP,
    .max_page   = KFS_NPG_DISK,
    .cur_dentry = 1,                                ///< dentry 0 always used by /
    .cur_inode  = 1,                                ///< inode 0 always used by /
    .cur_fmap   = 1,                                ///< fmap 0 never used
    .cur_page   = NPGU,                             ///< pages already used by BOOT and METADATA
    .bmp_dentry = { [0] = 1 },                      ///< dentry 0 always used by /
    .bmp_inode  = { [0] = 1 },                      ///< inode 0 always used by /
    .bmp_fmap   = { [0] = 1 },                      ///< fmap 0 never used
    .bmp_page   = {                                 ///< bmp_page <-- 11...10000000...0
        [0 ... ((NPGU-1)/8)-1] = 0xFF,              ///< ex. NPGU=26 ==> [0 ... 2]=0xFF
        [((NPGU+7)/8)-1] = (1<<(1+((NPGU+7)%8)))-1} ///< ex. NPGU=26 ==> [3] = 0b00000011
};
static kfs_dentry_t KfsDentry [KFS_MAX_DENTRY];     ///< up to MAX_DENTRY files
static kfs_inode_t  KfsInode [KFS_MAX_INODE] = {    ///< up to MAX_INODE files
    [0] = {.type=KFS_DIR, .mode=077}
};
static kfs_fmap_t   KfsFmap [KFS_MAX_FMAP];         ///< up to MAX_FMAP inode extensions

#ifdef _HOST_
static kfs_page_t KfsDisk[KFS_NPG_DISK];            ///< the whole disk is in memory only on Linux
#endif

static u16_t KfsOffset[KFS_MAX_INODE];              ///< last offset used for inode by read write

//--------------------------------------------------------------------------------------------------
// access functions
//--------------------------------------------------------------------------------------------------

int  kfs_root    (int dentry)     {return KfsDentry[dentry].root ;}
int  kfs_next    (int dentry)     {return KfsDentry[dentry].next ;}
int  kfs_leaf    (int dentry)     {return KfsDentry[dentry].leaf ;}
int  kfs_inode   (int dentry)     {return KfsDentry[dentry].inode;}
char *kfs_name   (int dentry)     {return KfsDentry[dentry].name ;}

int  kfs_isdir   (int dentry)     {return KfsInode[KfsDentry[dentry].inode].type == KFS_DIR;}

int  kfs_count   (int inode)      {return KfsInode[inode].count;}
int  kfs_type    (int inode)      {return KfsInode[inode].type; }
int  kfs_mode    (int inode)      {return KfsInode[inode].mode; }
int  kfs_size    (int inode)      {return KfsInode[inode].size; }
int  kfs_owner   (int inode)      {return KfsInode[inode].owner;}
int  kfs_mtime   (int inode)      {return KfsInode[inode].mtime;}
/*
 * FIXME there are restriction to change these properties
 */
int kfs_chmode  (int inode, int mode)  {return KfsInode[inode].mode = mode; }
int kfs_chowner (int inode, int owner) {return KfsInode[inode].owner= owner;}
int kfs_chmtime (int inode, int mtime) {return KfsInode[inode].mtime= mtime;}

//--------------------------------------------------------------------------------------------------
// static functions
//--------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------- stdlib

/**
 * \brief   get name from pathname up to character '/'
 *          pathname argument is not destroyed. Function NON-reentrant
 * \param   pathname has to be an absolute pathname, thus it must start with '/'
 *          kfs_getname ignores the last '/' at the end of pathname
 *          if pathname is NULL, then kfs_getname gives the next name in the previous pathname
 * \return  pointer to the current name or NULL on failure
 */
static char * kfs_getname (char *pathname)
{
    static char name[32] = "";                              // buffer for the extracted name
    static char *next = name;                               // next name, default empty name
    char * current = name;                                  // pointer used to fill the buffer

    if (pathname) {                                         // only true at first call
        if (*pathname != '/')                               // pathname must start with '/'
            return NULL;                                    // failure
        next = pathname;                                    // the next name is all pathname
    }
    if (*next == 0) return NULL;                            // no more name in pathname

    for (;*next && *next != '/'; *current++=*next++);       // strcpy till '/' excluded
    *current = 0;                                           // end of the name

    if (*next) next++;                                      // if *next == '/' go to next char
    while (*next == '/') next++;                            // several '/' are considered as single

    return name;                                            // at last return extracted name
}

/**
 * \brief   copy a string of characters
 * \param   dest is the address of the destination buffer
 * \param   src is the address of the source buffer
 * \return  nothing but the side effect on destination buffer
 */
static void  kfs_strcpy (char *dest, char *src)
{
    for (int i = 0; (i < KFS_MAX_NAME) && (*src); i++)
        *dest++ = *src++;
    *dest = 0;
}

/**
 * \brief   compare two strings of characters
 * \param   s1 is the first string
 * \param   s2 is the second string
 * \return  0 if s1 == s2 or the difference between s1 and s2 on the first character that differs
 */
static int kfs_strcmp(const char *s1, const char *s2)
{
    u8_t c1, c2;
    do {
        c1 = (u8_t) *s1++;
        c2 = (u8_t) *s2++;
    }
    while (c1 && (c1 == c2));
    return c1 - c2;
}

//--------------------------------------------------------------------------------------- allocators

/**
 * \brief   bitmap allocator
 * \param   bitmap is a bit array, but it is accessed by byte
 * \param   start is a pointer to position of last allocated or released bit (from 0 to max-1)
 * \param   max is the number of bits in the bitmap
 * \return  the position of the bit found or 0 if not found,
 *          there are 2 side effects, the bit bitmap[] at the returned position is cleared
 *          and start is updated for the next search
 */
static u32_t kfs_alloc_bitmap (u8_t bitmap[], u32_t *start, u32_t max)
{
    for (;*start != max ; *start += 1)                      // search from the last allocated
        if ((bitmap[*start/8] & (1 << (*start%8))) == 0) {  // if this bit is 0 (then usable)
            bitmap[*start/8] |= (1 << (*start%8));          // set it
            return *start;                                  // and return its postion
        }
    return 0;                                               // whenever no bit found
}

/**
 * \brief   releases one bit of a bitmap
 * \param   bitmap is a bit array, but it is accessed by byte
 * \param   start is a pointer to position of last allocated or released bit (from 0 to max-1)
 * \param   bit is the bit position to realease
 * \return  there are 2 side effects, the bit bitmap[] at the start position is set
 *          and start is updated for the next search
 */
static void kfs_free_bitmap (u8_t bitmap[], u32_t *start, u32_t bit)
{
    bitmap [bit/8] &= ~(1<<bit%8);                          // clear bit in bitmap
    if (bit < *start) *start = bit;                         // change position to start next time

}

/**
 * \brief   allocates a new dentry,
 *          dentry 0 is for '/' thus it is never free
 * \param   name of the new dentry
 * \return  On success, the new dentry, on failure, 0 (because dentry 0 is always allocated)
 */
static u32_t kfs_alloc_dentry (char *name)
{
    u32_t new= kfs_alloc_bitmap (KfsSblock.bmp_dentry, &KfsSblock.cur_dentry, KfsSblock.max_dentry);
    if (new == 0) return 0;                                 // no more dentries available
    kfs_strcpy (KfsDentry[new].name, name);                 // copy name
    return new;                                             // all other fields are not initialized
}

/**
 * \brief   frees a dentry (it is not necessary to erase its content, it will be done in allocation)
 * \param   dentry to free
 */
static void kfs_free_dentry (u16_t dentry)
{
    u16_t root = KfsDentry[dentry].root;                    // get the directory where dentry is
    u16_t curr = KfsDentry[root].leaf;                      // get the 1st entry of this directory
    u16_t prev = 0;                                         // initiate previous

    while (curr != dentry) {                                // while the current entry isn't dentry
        prev = curr;                                        // previous dentry becomes current
        curr = KfsDentry[curr].next;                        // get the next current
    }
    if (prev == 0)                                          // if previous==0, dentry was the 1st
        KfsDentry[root].leaf = KfsDentry[dentry].next;      // new 1st is the one just after dentry
    else                                                    // else
        KfsDentry[prev].next = KfsDentry[dentry].next;      // skip the dentry prev.next=dentry.next

    u32_t * pi = (u32_t *)&(KfsDentry[dentry]);             // erase data inside dentry
    for (int i = 0; i < 8; i++, *pi++=0)
    kfs_free_bitmap (KfsSblock.bmp_dentry, &KfsSblock.cur_dentry, dentry);  // free the bit
}

/**
 * \brief   allocates a new inode,
 *          inode 0 is for '/' thus it is never free
 * \param   type of the new inode
 * \return  On success, the new inode, on failure, 0 (because inode 0 is always allocated)
 */
static u16_t kfs_alloc_inode (u8_t type)
{
    u32_t new = kfs_alloc_bitmap (KfsSblock.bmp_inode, &KfsSblock.cur_inode, KfsSblock.max_inode);
    if (new == 0) return 0;                                 // no more dentries available
    KfsInode[new].type = type;                              // type could be chosen
    KfsInode[new].mode = 077;                               // 077 == RWXRWX
    KfsInode[new].count = 1;                                // new inode means only one reference
    return new;                                             // at last, return the new inode
}

/**
 * \brief   frees an inode (it is not necessary to erase it, it will be done in allocation)
 * \param   inode to free
 */
static void kfs_free_inode (int inode)
{
    u32_t * pi = (u32_t *)&(KfsInode[inode]);
    for (int i = 0; i < 8; i++, *pi++=0);
    kfs_free_bitmap (KfsSblock.bmp_inode, &KfsSblock.cur_inode, inode);
}

/**
 * \brief   allocates a new map,
 *          map 0 is never free
 * \return  On success, the new map, on failure, 0 (because fmap 0 is always allocated)
 */
static int kfs_alloc_fmap (void)
{
    return kfs_alloc_bitmap (KfsSblock.bmp_fmap, &KfsSblock.cur_fmap, KfsSblock.max_fmap);
}

/**
 * \brief   frees a map
 * \param   map to free
 */
static void kfs_free_fmap (u16_t fmap)
{
    u32_t * pi = (u32_t *)(KfsFmap[fmap]);
    for (int i = 0; i < 8; i++, *pi++=0);
    kfs_free_bitmap (KfsSblock.bmp_fmap, &KfsSblock.cur_fmap, fmap);
}

/**
 * \brief   allocates a new page of the disk
 * \return  On success, the new page, on failure, 0 (because page 0 is always allocated)
 */
static u16_t kfs_alloc_page (void)
{
    return kfs_alloc_bitmap (KfsSblock.bmp_page, &KfsSblock.cur_page, KfsSblock.max_page);
}

/**
 * \brief   frees a page of the disk
 * \param   page to free
 */
static void kfs_free_page (u16_t page)
{
    kfs_free_bitmap (KfsSblock.bmp_page, &KfsSblock.cur_page, page);
}

//-------------------------------------------------------------------------------------- disk access

static int  kfs_fmap (int inode)  {return KfsInode[inode].fmap; }

/*
 * returns the page number of the disk corresponding to the page n°pg_offset of a file.
 * - The 12 first pages are directly referenced by the inode, thus if file is smaller than 48kiB
 *   we have just to look in the inode (case [A])
 * - The following 16 are referenced in a fmap pointed by the inode,
 *   but olny if file size is smaller than 28 (12+16) pages (112kiB) (case [B])
 * - if the file is greater than 28 pages then the 1st level of map references a 2nd level
 *   of fmap and this 2nd level of fmap references pages, thus 16 x 16 pages (1MiB) (case [C])
 */
int kfs_page (int inode, int pg_offset)
{
    int size = kfs_size(inode);                             // real size of file
    if ((size==0) || (pg_offset > size>>12)) return -1;     // if pg_offset is > than the nb of page
    if (pg_offset < 12) return KfsInode[inode].page[pg_offset];// page is referenced by inode
    pg_offset -= 12;                                        // all others are in fmaps
    int map1 = kfs_fmap(inode);                             // get the first level
    if (size <= 28<<12) return KfsFmap[map1][pg_offset];    // if size is <= 28 pages, then 1 level
    int map2 = KfsFmap[map1][pg_offset/16];                 // else get the second  level
    return KfsFmap[map2][pg_offset%16];                     // then get the page
}

/**
 * returns the pointer to the box containing the page number of the disk corresponding to
 * the page n°pg_offset of a file. This box is in the inode or in the fmaps.
 * - The 12 first pages are directly referenced by the inode, thus if file is smaller than 48kiB
 *   we have just to look in the inode (case [A])
 * - The following 16 are referenced in a fmap pointed by the inode,
 *   but olny if file size is smaller than 28 (12+16) pages (112kiB) (case [B])
 * - if the file is greater than 28 pages then the 1st level of map references a 2nd level
 *   of fmap and this 2nd level of fmap references pages, thus 16 x 16 pages (1MiB) (case [C])
 * - if fmaps are not yet allocated, the function will do it
 *   We may have to create up to 2 maps (1st and 2nd level) and a page.
 *   There are many sub-cases depending on the current file size and the future size.
 */
static u16_t * kfs_ppage (u16_t inode, u16_t pg_offset)
{
    u32_t size = KfsInode[inode].size;              // previous size of file
    u16_t * pfmap2in1;                              // address of 2nd lev fmap in 1st lev fmap
    kfs_fmap_t * pfmap1;                            // 1st level of fmap address
    kfs_fmap_t * pfmap2;                            // 2nd level of fmap address
    u32_t fmap;                                     // 2nd level of fmap
    u32_t old_pg_size = size >> 12;                 // old last page offset of the file

    if (pg_offset < 12)                             // if inside 12 pages mapped by inode
        return &(KfsInode[inode].page[pg_offset]);  // -- get the address of page number

    if (old_pg_size < 12) {                         // if old size is <= 12 pages
        fmap = kfs_alloc_fmap();                    // -- alloc a new fmap
        if (fmap == 0) return NULL;                 // -- no more fmap
        KfsInode[inode].fmap = fmap;                // -- connect inode to the 1st level fmap
    }
    pg_offset -= 12;                                // shift of 12 for the pg_offset
    pfmap1 = &(KfsFmap[KfsInode[inode].fmap]);      // get the address of 1st level of fmap
    if (pg_offset < 16)                             // if new page offset is < 28
        return &((*pfmap1)[pg_offset]);             // -- get addr of page nbr in 1st level fmap

    if (old_pg_size < 28) {                         // if need to change from a 1 to 2 level fmap
        fmap = kfs_alloc_fmap();                    // -- new 1st level fmap
        if (fmap == 0) return NULL;                 // -- no more fmap
        KfsFmap[fmap][0] = KfsInode[inode].fmap;    // -- the old fmap that becomes 2nd level
        KfsInode[inode].fmap = fmap;                // -- connect inode to the 1st level fmap
        pfmap1 = &(KfsFmap[KfsInode[inode].fmap]);  // -- get the new addr of 1st level of fmap
    }
    pfmap2in1 = &((*pfmap1)[pg_offset/16]);         // get addr of 2nd fmap in 1st fmap
    if (*pfmap2in1 == 0) {                          // if 2nd level does not exit
        fmap = kfs_alloc_fmap();                    // -- new 2nd level of fmap
        if (fmap == 0) return NULL;                 // -- no more fmap
        *pfmap2in1 = fmap;                          // -- update le 1st level of fmap
    }
    pfmap2 = &(KfsFmap[*pfmap2in1]);                // get the address of 2nd level of fmap
    return &((*pfmap2)[pg_offset%16]);              // get addr of page nbr in 2nd level fmap
}

/**
 * \brief   Reads from the disk to the memory buffer.
 *          There are two cases : 1) if kfs runs on Linux 2) if kfs runs on ko6
 *          1) Linux: the kfs disk has been loaded in an array, so that's where we have to read it
 *          2) ko6:   the kfs disk is behind the disk device, so we need to ask the disk driver
 * \param   buf is a buffer address in memory
 * \param   page is a page number on disk, if page == 0 then the page is considered as full of 0
 * \return  0 if buf is filled with 0, 1 if the page read is not empty or -1 in case of failure
 *          If page read on disk in empty, but we return 1 it is not a problem
 */
static int kfs_read_page (int *buf, u16_t page)
{
    int a = 0;                                              // counter of words
    if (page == 0) {                                        // the page 0 is full of 0
        for (; a != 1024; a++, *(int *)buf++ = 0);          // fill buffer with 0
        return 0;                                           // success
    }
#ifdef _HOST_
    int * ppage = (int *)KfsDisk[page];               // ppage points to the first int of page
    for (; a != 1024; a++, *buf++ = *ppage++);
    return 1;                                               // success
#else
    return -1;                                              // FIXME  Real disk not yet handled
#endif
}

/**
 * \brief   writes to the disk from the memory buffer
 * \param   buf is a buffer address in memory
 * \param   page is a page number on disk
 * \return  0 if new page is full of 0, 1 if the page written is not empty or -1 in case of failure.
 *          If page written on disk in empty, but we return 1 it is not a problem
 */
static int kfs_write_page (void *buf, int page)
{
#ifdef _HOST_
    int res = 0;                                            // return value
    int a = 0;                                              // counter of words
    int * buffer = (int *)buf;                              // cast buf to int
    int * ppage = (int *)KfsDisk[page];                     // ppage points to the first int of page
    if ((page == 0) || (page >= KFS_NPG_DISK)) return -1;   // condition : 0 < page < KFS_NBPAGES
    for(;a != 1024; a++, res |= (*ppage++ = *buffer++));    // memory buf to disk page, compute res
    return (res) ? 1 : 0;                                   // if res remains 0 return 0 else 1
#else
    return -1;                                              // FIXME Real disk not yet handled
#endif
}

/**
 * \brief   Recursively executes the callback() function for all entries in a directory.
 *          This is a deep-first walk. The callback() function is not executed on the root.
 *          _cb_r means "call back recursive"
 *          This function must not be used directly, it is used by kfs_tree_cb()
 * \param   root has to be a dentry of a directory
 * \param   depth is the current depth of the walk
 * \param   callback is the callback function exectuted on each entry
 *          the call back function receives 3 arguments
 *          - dentry is the current entry in this directory which can be of any type
 *          - depth is the current depth of the walk (necessarily >= 1)
 *          - pos is the current position in the current direcrory
 * \return  the number of calls of callback()
 */
static int kfs_tree_cb_r (int root, int depth, void (*callback)(int dentry, int depth, int pos))
{
    int calls = 0;                                          // nb of calls at this depth
    int pos = 0;                                            // leaf position in current directory
    int leaf = kfs_readdir (root, root, NULL);              // root dentry has to be a directory
    if (leaf == -1) return 0;                               // if root is not a directory do nothing
    while (leaf) {                                          // while a leaf has been found
        callback (leaf, depth, pos++);                      // callback function execution
        calls++;                                            // increment the number of calls
        if (leaf && kfs_isdir(leaf))                        // if current leaf is a directory
            calls += kfs_tree_cb_r (leaf, depth+1,callback);// recursive call to kfs_tree_cb_r()
        leaf = kfs_readdir (root, leaf, NULL);              // get the next leaf in current root
    }
    return calls;
}

//--------------------------------------------------------------------------------------------------
// kfs API functions  (since they are external functions, usage is described in kfs.h)
//--------------------------------------------------------------------------------------------------

/**
 * "deep-first walk" of the name tree to find or create the file defined by its absolute name
 */
int kfs_open (char *pathname)                               // return dentry of absolute pathname
{
    char * name = kfs_getname (pathname);                   // get first name up to character '/'
    if (name == NULL) return -1;                            // pathname not correct, failure

    u32_t dentry = 0;                                       // search from root fs
    while ((name = kfs_getname (NULL)))                     // get the next name in pathname
        dentry = kfs_openat (dentry, name);                 // look for it in current directory

    KfsOffset[KfsDentry[dentry].inode] = 0;                 // by defaut, set offset to firstpage
    return dentry;                                          // finally, the last dentry found
}

/**
 * find or create the file defined by its relative name into the directory defined by its dentry
 */
int kfs_openat (int root, char *name)                       // return dentry of name in dir root
{
    u32_t leaf = KfsDentry[root].leaf;                      // get first leaf of current root
    int notfound = 1;                                       // by default, suppose name not found

    KfsInode[KfsDentry[root].inode].type = KFS_DIR;         // root is necessarily a directory

    if (leaf) {                                             // if there are leafs
        u32_t leaf_next = KfsDentry[leaf].next;             // get next leaf
        notfound = kfs_strcmp(KfsDentry[leaf].name, name);  // !=0 if different

        while (notfound && leaf_next) {                     // while not good leaf and there is more
            leaf = leaf_next;                               // change the leaf to the next leaf
            leaf_next = KfsDentry[leaf].next;               // get next leaf
            notfound= kfs_strcmp(KfsDentry[leaf].name,name);// compare current leaf
        }
    }

    if (notfound) {
        leaf = kfs_alloc_dentry (name);                     // create a new dentry for the file
        KfsDentry[leaf].inode = kfs_alloc_inode (KFS_FILE); // create an empty inode (default FILE)
        KfsDentry[leaf].leaf = 0;                           // if new leaf is FILE, thus no leaf
        KfsDentry[leaf].root = root;                        // set the root of the new leaf
        KfsDentry[leaf].next = KfsDentry[root].leaf;        // put new leaf in same level nodes list
        KfsDentry[root].leaf = leaf;                        // the beginning of the list of leafs
    }

    return leaf;                                            // return the found or created leaf
}

/**
 * readdir reads all entries in a directory
 */
int kfs_readdir (int root, int leaf, void *buf)
{
    if (!kfs_isdir(root)) return -1;                        // root dentry must be a directory

    if (buf) kfs_strcpy (buf, KfsDentry[leaf].name);        // if asked, get this leaf dentry name
    if (leaf == root)                                       // at first, leaf is the root
        return KfsDentry[root].leaf;                        // get the first leaf thus 0 if empty

    if (KfsDentry[leaf].root != root) return -1;            // leaf must be in root directory
    return KfsDentry[leaf].next;                            // find and return next leaf (0 if last)
}

/**
 * Try to read an entire page (4kiB) from file dentry
 */
int kfs_read (int dentry, int pg_offset, int *buf)
{
    int page = kfs_page (kfs_inode (dentry), pg_offset);     // get the page number in the disk
    if (page == -1) return 0;                               // if outside of the file
    return kfs_read_page (buf, page);                       // at last read disk page to buf
}

/*
* Sets the size field of the inode associated with the dentry to newsize.
* Returns the size of the file.
*/
int kfs_set_size(int dentry, int newsize)
{
    u16_t inode = kfs_inode(dentry);                // get inode associated with the dentry
    KfsInode[inode].size = newsize;                 // set the new size

    return kfs_size(inode);
}

/*
 * Try to write an entire buffer (4kiB) into file dentry, pages are placed in a 2-level map.
 * If the page already exists, it will be erased with the data from buf.
 * However, in the case where the page address is 0, a new page must be allocated.
 * Because if a page address is 0, it is considered full of 0 and it is not written to the disk.
 * We may have to create a page, if it does not exist yet, to fill it with the buffer.
 * If there are only zeros, then the page must be freed and the inode (or the map) where the page
 * is referenced must be modified to put 0 instead of the page number.
 */
int kfs_write (int dentry, int pg_offset, void *buf)
{
    u16_t inode = kfs_inode(dentry);                // get inode associated with the dentry
    u16_t *ppage = kfs_ppage (inode, pg_offset);    // get addr in fmaps of page number (alloc fmap)
    if (ppage == NULL) return -1;                   // NULL means no more space in fmaps
    if (*ppage == 0) *ppage = kfs_alloc_page();     // if page absent, allocate a new page
    if (*ppage == 0) return -1;                     // 0 means no more space on the disk
    if (kfs_write_page (buf, *ppage)) return 1;     // at last write disk page from buf
    kfs_free_page (*ppage);                         // if returns 0, it means page full of 0
    return *ppage = 0;                              // erase page number in fmap;
}

/*
 * FIXME add the test, do not work if dst_name exist
 */
int kfs_link (char *src_name, char *dst_name)
{
    u16_t src_dentry = kfs_open (src_name);
    u16_t dst_dentry = kfs_open (dst_name);
    u16_t src_inode  = KfsDentry[src_dentry].inode;
    u16_t dst_inode  = KfsDentry[dst_dentry].inode;
    KfsDentry[dst_dentry].inode = src_inode;
    KfsInode[src_inode].count++;
    kfs_free_inode (dst_inode);
    return dst_dentry;
}

/*
 * FIXME we need an open function without creation and test if it is a file, or use remove()
 */
int kfs_unlink (char *name)
{
    u16_t dentry = kfs_open (name);
    u32_t inode = KfsDentry[dentry].inode;
    u32_t size = KfsInode[inode].size;
    u32_t fmap1 = KfsInode[inode].fmap;
    u32_t fmap2 = 0;
    u32_t freemap=0;
    u16_t * ppage;

    kfs_free_dentry (dentry);

    if (KfsInode[inode].count-- == 0) {
        for (int s = size, p = 0; s > 0; s -= 1<<12, p++){
            if (p < 12) {
                ppage = &(KfsInode[inode].page[p]);
            } else if (size <= (28<<12)) {
                ppage = &(KfsFmap[fmap1][p-12]);
                if (p == size>>12) freemap=1;
            } else {
                fmap2 = KfsFmap[fmap1][(p-12)/16];
                ppage = &(KfsFmap[fmap2][(p-12)%16]);
                if ((p-12)%16 == 15) freemap=2;
                if (p == size>>12) freemap=3;
            }
            kfs_free_page (*ppage);
            if (freemap & 1) kfs_free_fmap(fmap1);
            if (freemap & 2) kfs_free_fmap(fmap2);
        }
        kfs_free_inode (inode);
    }
    return 0;
}

int kfs_tree_cb (int root, void (*callback)(int dentry, int depth, int postion))
{
    callback (root, 0, 0);                                  // callback function execution on root
    if (!kfs_isdir(root)) return 1;                         // if not a directory, it is finished
    return 1+kfs_tree_cb_r (root, 1, callback);             // else start the recursion
}

#ifdef  _HOST_

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int kfs_add_mbr(char* pathname)
{
    int nb_bytes_read;

    /* Open the file that contains the new mbr */
    int fd = open(pathname, O_RDONLY);
    if (fd < 0) {
        perror ("kfs_add_mbr: open");
        exit (1);
    }

    /* Clean the current mbr section */
    memset(&KfsMbr, 0, KFS_NPG_MBR<<12);

    /* Copy the new mbr on the disk in memory */
    nb_bytes_read = read(fd, &KfsMbr, 512);

    /* Check magic number */
    if (KfsMbr.magic != 0xD15C4C06) //FIXME : not sure that it's a good idea to add this test
    {
        printf("Warning : the mbr %s does not have the right magic number\n", pathname);
    }

    return nb_bytes_read;
}

int kfs_add_vbr(char* pathname)
{
    int nb_bytes_read;

    /* Open the file that contains the new vbr */
    int fd = open(pathname, O_RDONLY);
    if (fd < 0) {
        perror ("kfs_add_vbr: open");
        exit (1);
    }

    /* Clean the current vbr section */
    memset(KfsVbr, 0, KFS_NPG_VBR<<12);

    /* Copy the new vbr on the disk in memory */
    nb_bytes_read = read(fd, KfsVbr, KFS_NPG_VBR<<12);

    return nb_bytes_read;
}

int kfs_disk_load(char *pathname)
{
    ssize_t res = 0;
    int fd = open (pathname, O_RDONLY);
    if (fd < 0) {perror ("kfs_disk_load: open"); exit (1);}
    res |= read (fd, &KfsMbr, KFS_NPG_MBR<<12);
    res |= read (fd, KfsVbr, KFS_NPG_VBR<<12);
    res |= read (fd, &KfsSblock, KFS_NPG_SBLOCK<<12);
    res |= read (fd, KfsDentry, KFS_NPG_DENTRY<<12);
    res |= read (fd, KfsInode, KFS_NPG_INODE<<12);
    res |= read (fd, KfsFmap, KFS_NPG_FMAP<<12);
    res |= read (fd, KfsDisk + KFS_NPG_BOOT + KFS_NPG_META
                , (KFS_NPG_DISK - KFS_NPG_BOOT - KFS_NPG_META)<<12);
    if (res < 0) {perror ("kfs_disk_load: read"); exit (1);}
    close (fd);
    return res;
}

int kfs_disk_save(char *pathname)
{
    ssize_t res = 0;
    int fd = open (pathname, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
    if (fd < 0)  {perror ("kfs_disk_save: open"); exit (1);}
    res |= write (fd, &KfsMbr, KFS_NPG_MBR<<12);
    res |= write (fd, KfsVbr, KFS_NPG_VBR<<12);
    res |= write (fd, &KfsSblock, KFS_NPG_SBLOCK<<12);
    res |= write (fd, KfsDentry, KFS_NPG_DENTRY<<12);
    res |= write (fd, KfsInode, KFS_NPG_INODE<<12);
    res |= write (fd, KfsFmap, KFS_NPG_FMAP<<12);
    res |= write (fd, KfsDisk + KFS_NPG_BOOT + KFS_NPG_META
                 ,  (KFS_NPG_DISK - KFS_NPG_BOOT - KFS_NPG_META)<<12);
    if (res < 0) {perror ("kfs_disk_save: write"); exit (1);}
    close (fd);
    return res;
}
#endif
