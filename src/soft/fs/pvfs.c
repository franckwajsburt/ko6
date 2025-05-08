/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-19
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     pfs.c
  \author   Lili Zheng  
  \brief    

\*------------------------------------------------------------------------------------------------*/

#include <klibc.h>
#include "pvfs.h"

static void init_ofile(int fd, pfs_t *file, char flags, int type);

/**
 * Function that need to be called before using this imitating file system
 */
void pvfs_init (void)
{  
   kprintf ("coucou\n");
}

//--------------------------------------------------------------------------------------------------
// outils static
//--------------------------------------------------------------------------------------------------

// le noeud racine de ma structure
static struct pfs_s root = {
   .flags = RWX,
   .name = "/",
   .root = {0},
   .brothers = {0},
   .data = NULL
};

static struct pfs_s *currentDir; // need to be move to usermem structure (ask franck the permisison)

/**
 * charche le prochain mot qui precede par un '/'
 * par exemple si 'hello/char.txt', on a 'hello' dans le param word et
 *      retourne 5 en valeur retourné
 * retourne la taille du mot lue
 */
static int next_slash(const char *path, char *word)
{
    if (path == NULL || *path == 0)
    {
        return 0;
    }

    int i, j = 0;
    // skip all the meaningless /
    while (*path == '/')
        path++, j++;

    for (i = 0; (*(word + i) = *(path + i), *(path + i) != '\0'); i++)
    {
        if (*(path + i) == '/')
        {
            *(word + i) = '\0';
            break;
        }
    }

    // skip all the meaningless /
    while (*(path + i) == '/')
        i++;
    return i + j;
}

// /**
//  * \brief print the content of the current directory
//  */
// static void print_pfs(const struct pfs_s *dir)
// {

//     kprintf("'%s'\n", dir->name);

//     struct pfs_s *embedded;
//     list_foreach(&(dir->root), iterator)
//     {
//         embedded = list_item(iterator, struct pfs_s, brothers);
//         if (dir->root.prev != iterator)
//         {
//             kprintf(" ├── '%s'\n", embedded->name);
//         }
//         else
//         {
//             kprintf(" └── '%s'\n", embedded->name);
//         }
//     }
// }

/** init any already allocated pfs structur
 * * \param new is the pfs_s structure to be initialized
 * * \param name is the name of the file or directory
 * * \param flags is the permission of the file or directory
 * * \param type is the type of the file or directory or executable
 */
static void init_pfs(struct pfs_s *new, char *name, int flags, int type)
{
    list_init(&new->root);
    list_init(&new->brothers);
    new->flags = flags | type;
    strncpy(new->name, name, NAME_SIZE);
    new->name[NAME_SIZE - 1] = '\0';
    new->data = NULL;
    new->size = 0;
    new->parent = new; // deffault he is his own parent
}

/** create pfs structure */
static struct pfs_s *create_pfs(struct pfs_s *dir, char *name, int flags, int type)
{
    if (dir == NULL || name == NULL || *name == 0)
    {
        return NULL;
    }

    struct pfs_s *new = (struct pfs_s *)kmalloc(sizeof(struct pfs_s));
    if (new == NULL)
    {
        return NULL;
    }

    init_pfs(new, name, flags, type);
    new->parent = dir;
    list_addnext(&(dir->root), &(new->brothers));

    return new;
}

/**
 *  \brief destroy a pfs structure
 *  \param elem is the pfs_s structure to be destroyed
 *  \remarks this function is not recursive
 *        it is not possible to destroy the root directory
 *        it is not possible to destroy a directory that contains files
 */
static void destroy_dir(struct pfs_s **elem)
{
    // exception root
    if (strcmp((*elem)->name, "/") == 0)
        return;

    list_t *item_unused;

    if (IS_TYPE((*elem)->flags, DIR_T))

    {
        if (list_isempty(&((*elem)->root)) == 0)
        {
            kprintf("Error: directory '%s' is not empty\n", (*elem)->name);
            return;
        }
    }

    if ((*elem)->flags & EXEC_T)
    {
        kprintf("Error: directory '%s' is an executable \n", (*elem)->name);
    }

    item_unused = list_unlink(&((*elem)->brothers));
    item_unused = list_unlink(&((*elem)->root));

    kfree(item_unused);
    *elem = NULL;
}

/** Open any pfs from his aboslute name
 * do not touch any reference or create any stuct
 * without any side effect
 */
static struct pfs_s *open_any(const char *name)
{

    // BAD argument value
    if (name == NULL)
    {
        return NULL;
    }

    char pathname[PATH_MAX];
    char *last_slash = pathname;
    strncpy(pathname, name, PATH_MAX - 1);
    pathname[PATH_MAX - 1] = '\0';

    if (strcmp(last_slash, "/") == 0) // root case
        return &root;

    // last_slash normalisation
    int path_size = strlen(last_slash);
    if (last_slash[path_size - 1] == '/')
        last_slash[path_size - 1] = 0;

    struct pfs_s *embedded, *dir;
    char next[NAME_SIZE] = "/";

    // choose absolute or relative path
    if (last_slash[0] == '/')
    {
        dir = &root;
        embedded = &root;
    }
    else
    {
        dir = currentDir;
        embedded = currentDir;
    }

    int found = 0;
    // Cas general
    while (*last_slash != 0)
    {
        found = 0;

        // recherche dans le repertoire
        int next_size = next_slash(last_slash, next);
        last_slash = last_slash + next_size;

        if (*last_slash == 0 && strcmp("..", next) == 0)
        { // parent dir
            return dir->parent;
        }
        else if (*last_slash == 0 && strcmp(".", next) == 0)
        { // current dir
            return dir;
        }

        // named pfs
        list_foreach(&(dir->root), cur)
        {
            embedded = list_item(cur, struct pfs_s, brothers);

            // test if it's the file
            if (*last_slash == 0 && strcmp(embedded->name, next) == 0)
            {
                return embedded;
            }

            // change the directory
            if (strncmp(embedded->name, next, next_size) == 0)
            {
                dir = embedded;
                found = 1;
                break;
            }
            else if (strncmp("..", next, next_size) == 0)
            { // go to parent directory
                dir = dir->parent;
                found = 1;
                break;
            }
            else if (strncmp(".", next, next_size) == 0)
            { // stay in the same directory
                found = 1;
                break;
            }
        }

        // erreur de chemin
        if (found == 0)
        {
            return NULL;
        }
    }

    // trouvé
    return embedded;
}

/**
 * \return the file descriptor or -1 if no more available
 */
static int get_newFd()
{
    for (int i = 0; i < MAX_O_FILE; i++)
    {
        if (__usermem.o_file[i] == NULL){
            __usermem.o_file[i] = (struct file_s *) kmalloc(sizeof(struct file_s));
            return i;
        }
    }
    return -1;
}

/**
 * free a reference
 */
static struct file_s *put_Fd(int fd)
{
   struct file_s *res = __usermem.o_file[fd];
   res->ref --;

   if (res->ref == 0){
        kfree(res);
        res = NULL;
   }

   __usermem.o_file[fd] = NULL;
   return res;
}


//--------------------------------------------------------------------------------------------------
// pvfs API definition
//--------------------------------------------------------------------------------------------------

/**
 * fill the file descriptor table
 * create the file if he does not exist
 * \return the file descriptor or -1 if no more available
 */
int open(const char *pathname, int flags)
{
    // securité des argument
    if (pathname == NULL || *pathname == 0)
    {
        return -1;
    }
    struct pfs_s *file = open_any(pathname);
    if (file == NULL)
    { // create if not found
        // find last '/'
        int path_size = strlen(pathname);
        int i = 0;
        while (*(pathname + path_size - i) != '/' && *(pathname + path_size - i) != 0)
            i++;

        // prepare name
        char dir[PATH_MAX];
        char new_name[NAME_SIZE];
        struct pfs_s *directory;
        if (*(pathname) == '/')
        { // absolute path

            strncpy(dir, pathname, path_size - i);
            dir[path_size - i] = '\0';

            strncpy(new_name, pathname + path_size - i + 1, NAME_SIZE);
            new_name[NAME_SIZE - 1] = 0;

            if (*(dir) == 0)
            { // root case
                directory = &root;
            }
            else
            { // basic case
                directory = open_any(dir);
                if (directory == NULL || (IS_TYPE(directory->flags, DIR_T)))
                {
                    return -1;
                }
            }
        }
        else
        { // relative path
            directory = currentDir;
            strncpy(new_name, pathname, NAME_SIZE);
            new_name[NAME_SIZE - 1] = 0;
        }

        // create the file, is put by deffaul on read write
        file = create_pfs(directory, new_name, RW, FILE_T);
        if (file == NULL)
            return -1;
    }

    if (file && (IS_TYPE(file->flags, FILE_T)))
    {
        int fd = get_newFd();
        // __usermem.o_file[fd] = 
        init_ofile(fd, file,flags,FILE_T);
        return fd;
    }

    if (file && (IS_TYPE(file->flags, DIR_T)))
    {
        int fd = get_newFd();
        // __usermem.o_file[fd] = 
        init_ofile(fd, file,flags,DIR_T);
        return fd;
    }
    return -1;
}

// make place in the file descriptor table
int close(int fd)
{
    // verifie que fd est valide
    if (fd < 0 || fd >= MAX_O_FILE)
    {
        return -1;
    }
    put_Fd(fd);
    __usermem.o_file[fd] = NULL;
    return 0;
}

int file_write(int fd, char *buf, unsigned count)
{
    // securité des arguments
    if (fd < 0 || fd >= MAX_O_FILE || buf == NULL || count > PAGE_SIZE)
    {
        return -1;
    }

    // verifie que le fd est valide
    struct pfs_s *file = __usermem.o_file[fd]->file;
    if (file == NULL || (IS_TYPE(file->flags, DIR_T)))
    {
        return -1;
    }

    if (file->data == NULL)
    {
        file->data = kmalloc(PAGE_SIZE);
        if (file->data == NULL)
        {
            return -1;
        }
        file->size = 0;
    }

    memcpy(file->data, buf, count);
    return count; // no way to verify
}

/**
 * \brief read a file
 * \param fd is the file descriptor
 * \param buf is the buffer to put the data
 * \param count is the size of the buffer
 */
int file_read(int fd, char *buf, unsigned count)
{
    // securité des arguments
    if (fd < 0 || fd >= MAX_O_FILE || buf == NULL || count > PAGE_SIZE)
    {
        return -1;
    }

    struct pfs_s *file = __usermem.o_file[fd]->file;
    if (file == NULL || !(IS_TYPE(file->flags, FILE_T)) || file->data == NULL)
    {
        return -1;
    }

    memcpy(buf, file->data, count); // no way to know if he failed

    return count;
}


/**
 * only destroy
 * since we don't have counter reference we don't know if someone is working with the directory
 * the user have to be carefull (not good pratice to relie on user, need to be fixed)
 */
int unlink(const char *pathname){
    pfs_t* elem =open_any(pathname);

    list_t *item_unused = list_unlink(&((elem)->brothers));
    item_unused = list_unlink(&((elem)->root));

    kfree(item_unused);
    return 0;
}



//--------------------------------------------------------------------------------------------------
// dirent API definition
//--------------------------------------------------------------------------------------------------

void rewinddir(DIR *dirp)
{
    dirp->item = dirp->root;
}

struct dirent_s *readdir(DIR *dirp)
{
    if (dirp == 0)
    {
        return NULL;
    }

    struct dirent_s *res = dirp;
    if (res == NULL)
    {
        return NULL;
    }

    // return null if end of stream
    struct pfs_s *dir = res->root;
    struct pfs_s *cur = res->item;
    list_t *next;

    if (dir == cur)
    { // premier cas le decalage est different
        next = cur->root.next;
    }
    else
    {
        next = cur->brothers.next;
    }

    if (next == &dir->root)
    { // si on revient au debut
        return NULL;
    }

    cur = list_item(next, pfs_t, brothers);
    res->item = list_item(&cur->brothers, pfs_t, brothers);
    return res;
}

/**
 * \brief change the current directory
 * \param pathname is the absolute name of the file or directory to find in root
 * \return 0 if success, -1 if error
 */
int chdir(const char *pathname)
{
    // check arguments validity (put it for ko6)
    // if ( !pathname){
    //     return -1;
    // }

    if (*pathname == 0)
    { // fake HOME directory
        currentDir = &root;
        return 0;
    }

    struct pfs_s *embedded = open_any(pathname);

    // Verifie que c'est un repertoire
    if (embedded && (IS_TYPE(embedded->flags, DIR_T)))
    {
        currentDir = embedded;
        return 0;
    }

    return -1;
}

/**
 * only destroy if it is empty
 * since we don't have counter reference we don't know if someone is working with the directory
 * the user have to be carefull (not good pratice to relie on user, need to be fixed)
 */
int rmdir(const char *pathname){
    pfs_t* dir =open_any(pathname);
    destroy_dir(&dir);
    return 0;
}



//--------------------------------------------------------------------------------------------------
// file_s API definition
//--------------------------------------------------------------------------------------------------

// Noop function

int no_read(int fd, char *buf, unsigned count){
    return 0;
}

int no_write(int fd, char *buf, unsigned count){
    return count;
}

int dup2(int oldfd,int newfd){
    put_Fd(oldfd);
    O_FILE[newfd]->ref++;
    O_FILE[oldfd]= O_FILE[newfd];
    return 0;
}

/**
 * Init a open file struture and put the right function
 */
static void init_ofile(int fd, pfs_t *file, char flags, int type){
    __usermem.o_file[fd]->file =file;
    __usermem.o_file[fd]->flags=flags;
    
    switch (flags & 0xF8)
    {
    case DIR_T:
        __usermem.o_file[fd]->read = no_read;
        __usermem.o_file[fd]->write = no_write;
        break;
    
    case FILE_T:
        __usermem.o_file[fd]->read = file_read;
        __usermem.o_file[fd]->write = file_write;
        break;
        
    case STDIN_T:
        __usermem.o_file[fd]->read = tty_read;
        __usermem.o_file[fd]->write = tty_write;
        break;

    case STDOUT_T:
        __usermem.o_file[fd]->read = tty_read;
        __usermem.o_file[fd]->write = tty_write;
        break;
    
    case STDERR_T:
        __usermem.o_file[fd]->read = tty_read;
        __usermem.o_file[fd]->write = tty_write;
        break;

    default:
        __usermem.o_file[fd]->read = no_read;
        __usermem.o_file[fd]->write = no_write; 
        break;
    }


}




/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
