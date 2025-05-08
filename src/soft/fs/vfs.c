/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-05-03
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     fs/vfs.c
  \author   Franck Wajsburt
  \brief    Virtual File System

\*------------------------------------------------------------------------------------------------*/

#include <klibc.h>

int V=0;

void vfs_test(void)
{
    kprintf("[VFS TEST] Start\n");

    // 1. Lire le répertoire racine
    vfs_inode_t *root = vfs_resolve(NULL, "/");
    if (!root) {
        kprintf("[VFS TEST] Failed to resolve root inode\n");
        return;
    }
    vfs_inode_t *inode = vfs_resolve (NULL,"/");
    VAR (%x\n,inode);
/*
    vfs_dirent_t entry;
    kprintf("[VFS TEST] Directory listing of '/':\n");
    while (vfs_readdir(root, &entry) == 0) {
        kprintf(" - %s\n", entry.name);
    }

    // 2. Résoudre un fichier
    const char *path = "/hello.txt";
    vfs_inode_t *inode = vfs_resolve(NULL, path);
    if (!inode) {
        kprintf("[VFS TEST] File '%s' not found\n", path);
        return;
    }

    // 3. Ouvrir le fichier
    vfs_file_t *file = vfs_file_open(inode);
    if (!file) {
        kprintf("[VFS TEST] Failed to open file '%s'\n", path);
        return;
    }

    // 4. Lire le contenu
    char buf[128];
    int len = vfs_read(file, buf, sizeof(buf) - 1);
    buf[len] = '\0';
    kprintf("[VFS TEST] Contents of '%s':\n%s\n", path, buf);

    vfs_file_close(file);
*/
    kprintf("[VFS TEST] End\n");
}

//--------------------------------------------------------------------------------------------------
// General VFS initialisation
//--------------------------------------------------------------------------------------------------

hto_t *Vfs_icache;                                          ///< inode cache
list_t Vfs_icache_lru;                                      ///< list of not referenced inode 

static int vfs_icache_init (size_t nbentries);              // defined bellow
errno_t vfs_init (void)
{
    errno_t err = vfs_filesystem_register (&fs1_ops);       // Register the filesystem type first
    if (err != SUCCESS && err != -EEXIST) return err;

    superblock_t *sb = vfs_superblock_alloc ();             // Create a new superblock
    if (!sb) return -ENOMEM;

    blockdev_t *bdev = blockdev_get (0);                    // default block dev 0 for partition '/'
    if (!bdev) { kfree (sb); return -ENODEV; }

    err = vfs_mount ("/", sb, bdev, &fs1_ops);              // root '/' fs --> create root inode
    if (err != 0) { kfree (sb); return err; }

    err = vfs_icache_init (512);                            // 509 inodes (must be < 4KiB)
    if (err != 0) { kfree (sb); return err; }
    
    INFO ("vfs_init: '/' (fs1) successfully mounted on block device 0");
    return SUCCESS;
}

//--------------------------------------------------------------------------------------------------
// VFS API exposed to syscall layer
//--------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------- file system API

#define VFS_FILESYSTEM_MAX 8                                ///< Max nb of supported filesystems

static const vfs_fs_type_t *fs_registry[VFS_FILESYSTEM_MAX];///< Static registry of filesystem types

const vfs_fs_type_t *vfs_filesystem_get (const char *name)
{
    ASSERT (V,"name %s", name);
    if (!name) return NULL;                                 // nothing to search
    for (int i = 0; i < VFS_FILESYSTEM_MAX; ++i) {          // browse the registery table
        const vfs_fs_type_t *fs = fs_registry[i];           // get the registered fs
        if (fs && strcmp (fs->name, name) == 0)             // if there is a fs and name found
            return fs;                                      // return the fs 
    }
    return NULL;                                            // not found
}

errno_t vfs_filesystem_register (const vfs_fs_type_t *ops)
{
    ASSERT (V,"ops %x", ops);
    if (!ops || !ops->name) return -EINVAL;                 // check argument validity
    if (vfs_filesystem_get (ops->name)) return -EEXIST;     // already registered
    for (int i = 0; i < VFS_FILESYSTEM_MAX; ++i) {          // Find a free slot
        if (fs_registry[i] == NULL) {                       // free slot found
            fs_registry[i] = ops;                           // register ops
            return SUCCESS;                                 // success
        }
    }
    return -ENOSPC;                                         // registry full
}

//-------------------------------------------------------------------------------------- file mapper

/**
 * FIXME To be implemented
 */
errno_t vfs_mapping_destroy (vfs_inode_t *inode)
{
    (void)inode;
    return SUCCESS;
}

//---------------------------------------- mounted file system tables: definition & helper functions

#define VFS_MOUNT_MAX 15                                    ///< Max number of mounted filesystems

struct mount_point_s {
    char *path;                                             ///< absolut path where to mount: "/mnt"
    vfs_inode_t *inode;                                     ///< mount point inode in parent sb
    superblock_t *sb;                                       ///< mounted file system
} Vfs_mount_table [VFS_MOUNT_MAX];                          ///< global mount table
mnt_id_t Vfs_mount_max;                                     ///< actual mounted file system number

#define IS_VALID_MNT_ID(id) ((id)>=0 && (id)<Vfs_mount_max) ///< Check mount id validity

/// Iterate over all mount IDs in Vfs_mount_table[]
/// This macro abstracts away the implementation detail (array vs future dynamic structure)
#define vfs_mount_foreach_id(id)     for (mnt_id_t id = 0; id < Vfs_mount_max; ++id)
#define vfs_mount_foreach_id_all(id) for (mnt_id_t id = 0; id < VFS_FILESYSTEM_MAX; ++id)

/**
 * \brief Vfs_mount_table accessors which hide the internal structure of the Vfs_mount_table
 */
static char *vfs_mount_path_get (mnt_id_t id) {
    return IS_VALID_MNT_ID(id) ? Vfs_mount_table[id].path : NULL;
}
static void vfs_mount_path_set (mnt_id_t id, char *path) {
    if (IS_VALID_MNT_ID(id)) Vfs_mount_table[id].path = path;
}
static vfs_inode_t *vfs_mount_inode_get (mnt_id_t id) {
    return IS_VALID_MNT_ID(id) ? Vfs_mount_table[id].inode : NULL;
}
static void vfs_mount_inode_set (mnt_id_t id, vfs_inode_t *inode) {
    if (IS_VALID_MNT_ID(id)) Vfs_mount_table[id].inode = inode;
}
static superblock_t *vfs_mount_sb_get (mnt_id_t id) {
    return IS_VALID_MNT_ID(id) ? Vfs_mount_table[id].sb : NULL;
}
static void vfs_mount_sb_set (mnt_id_t id, superblock_t *sb) {
    if (IS_VALID_MNT_ID(id)) Vfs_mount_table[id].sb = sb;
}

/**
 * \brief Update the greater mnt_id used for Vfs_mount_table
 * \param id the mnt_id just modified
 * \return nothing but a possible change of the global variable Vfs_mount_max
 */
static void vfs_mount_update_max (mnt_id_t id) {
    if (id >= Vfs_mount_max)                                    // a new place -> max increments
        Vfs_mount_max = id+1;                                   // max is a number of places -> +1
    else if (id < Vfs_mount_max) {                              // if it a freeing
        mnt_id_t i = Vfs_mount_max-1;                           // begin search for the actual max
        while ((i>=0) && (Vfs_mount_table[i].path == NULL)) i--;// search until non-NULL or end
        Vfs_mount_max = i+1;                                    // new max found
    }
}

/**
 * \brief allocate a new entry into the Vfs_mount_table
 * \return a free mnt_id or -ENOSPC if no space left
 */
static errno_t vfs_mount_alloc (void)
{
    vfs_mount_foreach_id_all (id) {                         // browse the entire mount table
        if (vfs_mount_path_get (id) == NULL) {              // empty place found
            vfs_mount_update_max (id);                      // update the mounted fs max  
            return id;                                      // return the mnt_id in mount table
        }
    }
    return -ENOSPC;                                         // place or path not found
}

/**
 * \brief Registers a superblock mounted under a given path.
 * \param path  Path of the mount point (e.g., “/mnt”, “/”)
 * \param sb    Superblock to register
 * \return mnt_id in the table or -ENOSPC if failed
 */
static errno_t vfs_mount_register (const char *path, vfs_inode_t *inode, superblock_t *sb)
{
    ASSERT (V,"path %s inode %x sb %x",path,inode,sb);
    mnt_id_t id = vfs_mount_alloc ();                       // get a place in mount table
    if (id < 0) return -ENOSPC;                             // no space left
    sb->mnt_id = id;                                        // sb identifier in the mount table
    vfs_mount_path_set (id, kstrdup(path));                 // register path 
    vfs_mount_inode_set(id, inode);                         // register mnt point inode in parent sb
    vfs_mount_sb_set   (id, sb);                            // register superblock
    return id;                                              // success
}

/**
 * \brief Unregisters a mounted superblock
 * \param path  Path of the mount point (e.g., “/mnt”, “/”)
 * \return the superblock previously mounterd or NULL if path not found
 */
static superblock_t *vfs_mount_unregister (const char *path)
{
    mnt_id_t id = vfs_mount_lookup (path);                  // retreive the place in mount table
    if (id < 0) return NULL;                                // path not found 
    superblock_t *sb = vfs_mount_sb_get (id);               // retreive the sb
    kfree (vfs_mount_path_get (id));                        // free path name
    vfs_mount_path_set (id, NULL);                          // unregister path 
    vfs_mount_inode_set (id, NULL);                         // unregister inode
    vfs_mount_sb_set (id, NULL);                            // unregister superblock
    vfs_mount_update_max (id);                              // update the max actual mounted fs
    sb->mnt_id = -1;                                        // superblock is unregistered
    return sb;                                              // ret superblock previously registered
}

//---------------------------------------------------------- superblock management / mount / unmount

superblock_t *vfs_superblock_alloc (void) 
{
    superblock_t *sb = kmalloc (sizeof(superblock_t));      // new superblock
    if (!sb) return NULL;                                   // no space left
    sb->mnt_id = -1;                                        // unregistered by default
    return sb;
}

errno_t vfs_kern_mount (superblock_t *sb, blockdev_t *bdev, const vfs_fs_type_t *ops)
{
    ASSERT (V,"sb %x bdev %x ops %x", sb, bdev, ops);
    if (!sb || !bdev || !ops) return -EINVAL;               // check arguments validity
    sb->ops = ops;                                          // assign the filesystem operations
    sb->bdev = bdev;                                        // assign the block device
    sb->fs_data = NULL;                                     // will be init by fs-specific mount
    return ops->mount (sb, bdev);                           // call fs-specific mount function
}

errno_t vfs_kern_unmount (superblock_t *sb)
{
    if (!sb || !sb->ops || !sb->ops->mount) return -EINVAL; // check arguments validity
    return sb->ops->unmount (sb);                           // Call the fs-specific mount function
}

errno_t vfs_mount (const char *path, superblock_t *sb, blockdev_t *bdev, const vfs_fs_type_t *ops)
{
    ASSERT (V,"path %s sb %x bdev %x ops %x", path, sb, bdev, ops);
    vfs_inode_t *inode = vfs_resolve (NULL, path);          // retreive path's inode BEFORE mounting
    VAR(%x\n,inode);
    int ret = vfs_kern_mount (sb, bdev, ops);               // mount the new file system
    if (ret >= 0) {
        ret = vfs_mount_register (path, inode, sb);         // find a place in vfs_mount_table
        if (ret < 0)
            vfs_kern_unmount (sb);                          // rollback mount if registration failed
    }
    return ret;                                             // return < 0 on fealure
}

errno_t vfs_umount (const char *path)
{
    superblock_t *sb = vfs_mount_unregister (path);         // unregister from the Vfs_mount_table
    return vfs_kern_unmount (sb);                           // Call the fs-specific mount function
}

mnt_id_t vfs_mount_lookup (const char *path)
{
    vfs_mount_foreach_id (id) {                             // browse the mount table
        char *curpath = vfs_mount_path_get (id);            // get the current path
        if (path && (strcmp (path, curpath) == 0))          // path found
            return id;                                      // return the mnt_id in mount table
    }
    return -ENOENT;                                         // place or path not found
}

superblock_t *vfs_mount_resolve (const char *path)
{
    ASSERT(V,"path %s", path);
    if (!path || path[0] != '/') return NULL;               // path has to be absolute

    superblock_t *best_sb = NULL;                           // will the the best superblock
    int best_len = -1;                                      // the one with the greater path length

    vfs_mount_foreach_id (id) {                             // browse the mount table
        const char *mnt = vfs_mount_path_get (id);          // get the current mount path
        superblock_t *sb = vfs_mount_sb_get (id);           // and its superblock
        if (!mnt || !sb) continue;                          // if nothing registered continue
        int len = strlen (mnt);                             // get the mount path length
        if (len > best_len &&                               // the current length is greater
            strncmp (path, mnt, len) == 0 &&                // and the path begin with mnt
            (path[len] == '/' || path[len] == '\0')) {      // and the path is not badly cut
            best_sb = sb;                                   // then sb is yhe new best choice
            best_len = len;                                 // for that length
        }
    }
    VAR(%x\n, best_sb);
    return best_sb;                                         // return the best superblock
}

//---------------------------------------------------------------------------- lookup / open / close

vfs_inode_t *vfs_resolve (vfs_inode_t *base, const char *path)
{
    ASSERT(V,"base %p path %s", base, path);
    if (!path || path[0] == '\0') return NULL;              // nothing to resolve

    superblock_t *sb;
    vfs_inode_t *inode;

    char *path_copy = kmalloc(PAGE_SIZE);                   // allocate temporary buffer
    strncpy (path_copy, path, PAGE_SIZE);                   // duplicate path
    path_copy[PAGE_SIZE - 1] = '\0';                        // ensure null-termination

    char *parts[32];                                        // max 32 path segments
    int count = strsplit (path_copy, "/", parts, 32);       // split path into components

    if (path[0] == '/') {
        if (base != NULL) goto resolve_fail;                // inconsistent: absolute path + base
        sb = vfs_mount_resolve (path);                      // find the matching superblock
        if (!sb || !sb->root) goto resolve_fail;            // no root found
        inode = sb->root;
    } else {
        if (base == NULL) goto resolve_fail;                // relative path needs a base inode
        sb = base->sb;
        inode = base;
    }
    VAR(%d\n,count);
    for (int i = 0; i < count; ++i) {
        if (parts[i][0] == '\0' || strcmp(parts[i], ".") == 0) continue;  // skip empty or "."

        inode = sb->ops->lookup(sb, inode, parts[i]);       // lookup next component in real FS
        if (!inode) goto resolve_fail;

        vfs_mount_foreach_id(id) {                          // Check if this inode is a mount point
            superblock_t *mnt_sb = vfs_mount_sb_get(id);    // retrieve superblock from mount table
            if (mnt_sb && vfs_mount_inode_get(id) == inode){// match found: inode is a mount root
                sb = mnt_sb;                                // switch to mounted filesystem
                inode = sb->root;                           // reset to its root
                break;
            }
        }
    }
    kfree(path_copy);                                       // at last free the patah copy
    return inode;                                           // and return the resolution result

resolve_fail:                                               // if something wrong happened
    kfree(path_copy);                                       // free the path copy
    return NULL;                                            // an return NULL
}

vfs_file_t *vfs_open (vfs_inode_t *base, const char *path)
{
    vfs_inode_t *inode = vfs_resolve (base, path);          // retrieve the path's inode 
    if (!inode) return NULL;
    vfs_file_t *file = kmalloc (sizeof(vfs_file_t));        // allocate a new file 
    if (!file) { vfs_inode_release (inode); return NULL; }  
    vfs_inode_get (inode);                                  // increase refcount because of the file
    file->inode = inode;                                    // iniatialize file
    file->offset = 0;                                       // start file access from the beginning
    return file;                                            // at last, return the new file
}

errno_t vfs_close (vfs_file_t *file)
{
    vfs_inode_release (file->inode);
    kfree (file);
    return SUCCESS;
}

vfs_file_t *vfs_opendir (vfs_inode_t *base, const char *path)
{
/*
    vfs_inode_t *inode = vfs_resolve (base, path);          // retrieve the path's inode 
    if (!inode || !vfs_inode_is_dir(inode)) return NULL;
    vfs_file_t *file = kmalloc(sizeof(vfs_file_t));
    if (!file) return NULL;
    file->inode = inode;
    file->offset = 0;
    return file;
*/
    return NULL;
}

//-------------------------------------------------------------------- read / write / seek / readdir

errno_t vfs_read (vfs_file_t *file, void *buffer, size_t size)
{
    if (!file || !buffer || !size) return -EINVAL;                    // check arguments validity
    vfs_inode_t *inode = file->inode;                                 // retreive file's vfs_inode
    if (!inode||!inode->sb||!inode->sb->ops||!inode->sb->ops->read)   // check structures
        return -EINVAL;
    int ret = inode->sb->ops->read(inode, buffer, file->offset, size);// Perform the read
    if (ret < 0) return ret;
    file->offset += ret;                                              // Advance the file offset
    return ret;
}

errno_t vfs_write (vfs_file_t *file, const void *buffer, size_t size)
{
    (void)file;
    (void)buffer;
    (void)size;
    return -ENOSYS;
}

errno_t vfs_seek (vfs_file_t *file, int offset, int whence)
{
    unsigned size;
    switch (whence) {
    case SEEK_SET:
        file->offset = offset;
        return SUCCESS;
    case SEEK_CUR:
        file->offset += offset;
        return SUCCESS;
    case SEEK_END:
        size = file->inode->size;
        file->offset = size + offset;
        return SUCCESS;
    default:
        return FAILURE;
    }
}

vfs_dirent_t *vfs_readdir (vfs_file_t *dir)
{
/*
    if (!dir || !dir->inode || !dir->inode->sb || !dir->inode->sb->ops) return NULL;
    const vfs_fs_type_t *ops = dir->inode->sb->ops;                 // retreive the fs API
    if (!ops->readdir) return NULL;                                 // fs does not support readdir

    vfs_dirent_t *entry = NULL;                                     // allocated by ops->readdir()
    errno_t err = ops->readdir (dir->inode, dir->offset, &entry);   // call the real fs readir
    if (err == SUCCESS && entry != NULL) {                          // there is a entry
        dir->offset++;                                              // advance to next entry
        return entry;                                               // HAVE TO BE FREED AFTER USE!
    }
*/
    return NULL;                                                    // no more entries or error
}

//-------------------------------------------------------------------------------- getattr / setattr

errno_t vfs_getattr (vfs_inode_t *inode, struct stat *statbuf)
{
    (void)inode;
    (void)statbuf;
    return -ENOSYS;
}

errno_t vfs_setattr (vfs_inode_t *inode, const struct stat *statbuf)
{
    (void)inode;
    (void)statbuf;
    return -ENOSYS;
}

//--------------------------------------------------------------------------------------------------
// dentry API
//--------------------------------------------------------------------------------------------------

vfs_dentry_t *vfs_dentry_lookup (vfs_dentry_t *parent, const char *name)
{
    (void)parent;
    (void)name;
    return NULL;
}

vfs_dentry_t *vfs_dentry_create (vfs_dentry_t *parent, const char *name, vfs_inode_t *inode)
{
    (void)parent;
    (void)name;
    (void)inode;
    return NULL;
}

void vfs_dentry_destroy (vfs_dentry_t *dentry)
{
    (void)dentry;
}

//--------------------------------------------------------------------------------------------------
// inode API
//--------------------------------------------------------------------------------------------------

#define INO_KEY(mnt_id,ino) ((void *)((unsigned long)((mnt_id << 28)| (ino & 0x0FFFFFFF))))
#define INODE_KEY(inode)    INO_KEY((inode)->sb->mnt_id,(inode)->ino)

/**
 * \brief Initialize icache
 * \param nbentries maximum number of working inodes
 * \return SUCCESS or -ENOMEM, if there is not enough memory
 * FIXME improve comments
 */
static int vfs_icache_init (size_t nbentries) 
{
    Vfs_icache = hto_create (nbentries, 1);                     // max inodes 
    if (!Vfs_icache) return -ENOMEM;                            // impossible to create icache
    list_init (&Vfs_icache_lru);                                // initialize releasable inode list
    return SUCCESS;                                             // success
}
    
/**
 * \brief Evict a VFS inode and release all associated resources.
 *        This function must only be called for inodes with refcount == 0,
 *        and after they have been removed from the inode cache.
 *        It releases:
 *        - the filesystem-specific data (via fs->destroy_inode),
 *        - the file mapping if any (e.g. directory entries, page cache),
 *        - and the inode structure itself.
 * \param inode Pointer to the vfs_inode_t to destroy.
static void vfs_icache_evict (vfs_inode_t *inode)
{
    if (!inode) return;
    PANIC_IF (inode->refcount, "inode still referenced");
    hto_del (Vfs_icache, INODE_KEY(inode)); 
    inode->sb->ops->evict (inode);                          // Call the fs-specific destroy fun
    if (inode->mapping)                                     // Destroy file mapping if present
        vfs_mapping_destroy (inode);                // To be implemented
    // TODO: release associated dentries when dentry cache is implemented
    // Free the inode itself
    kfree(inode);
}
 */

/**
 * \brief Insert an inode into the global VFS inode cache.
 *        If the cache is full, this function evicts the oldest unused inode (refcount == 0) 
 *        from the LRU list to make space. The victim is removed from the cache and is eviscted
 *        with vfs_inode_evict(). The given inode is inserted or the system panics.
 * \param inode Pointer to the vfs_inode_t to insert.
static void vfs_icache_insert(vfs_inode_t *inode)
{
    void *key = INODE_KEY(inode);
    while (hto_set (Vfs_icache, key, inode) < 0) {              // try to insert the inode
        list_t *victim = list_getlast (&Vfs_icache_lru);        // if no space, get the lru
        PANIC_IF (!victim, "icache full: no evictable inode");  // too much file/dir openened
        list_unlink (victim);                                   // unlink victim from the lru list
        vfs_inode_t *inode = list_item(victim,vfs_inode_t,list);// retreive inode from list pointer
        vfs_icache_evict (inode);                               // remove that inode from icache 
    }
}
 */

/**
 * \brief Lookup an inode in the VFS inode cache.
 * \param sb  Pointer to the superblock where the inode should belong.
 * \param ino Index of the inode to search for.
 * \return Pointer to the vfs_inode_t if found, NULL otherwise.
static vfs_inode_t *vfs_icache_lookup(superblock_t *sb, ino_t ino)
{
    return hto_get (Vfs_icache, INO_KEY(sb->mnt_id, ino));         
}
 */

vfs_inode_t *vfs_inode_create (superblock_t *sb, ino_t ino, size_t size, mode_t mode, void *data)
{
    vfs_inode_t *inode = kmalloc (sizeof (vfs_inode_t));     // allocate a new vfs_inode
    if (!inode) return NULL;
    inode->sb = sb;
    inode->ino = ino;
    inode->size = size;
    inode->mode = mode;
    inode->refcount = 1;
    inode->flags = 0;
    inode->data = data;
    inode->mapping = NULL;
    inode->dentries = NULL;
    list_init (&inode->list);
    return inode;
}

vfs_inode_t *vfs_inode_lookup (superblock_t *sb, ino_t ino) 
{
    return NULL;
}

void vfs_inode_get (vfs_inode_t *inode)                     // FIXME : should be atomic
{
    inode->refcount++;                                      // there is another reference
}

void vfs_inode_release(vfs_inode_t *inode)
{
    if (!inode) return;
    PANIC_IF (inode->refcount == 0, "refcount already 0");
    if (--(inode->refcount) == 0)                           // decrement ref nb, if 0 then
        list_addfirst (&Vfs_icache_lru, &inode->list);      // add inode to the releasable inodes
}

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
