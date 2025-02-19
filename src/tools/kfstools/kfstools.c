/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date        2023-04-09
  | / /(     )/ _ \     \copyright   2023 Sorbonne University
  |_\_\ x___x \___/                  https://opensource.org/licenses/MIT

  \file     kfstools.c
  \author   Franck Wajsburt
  \author   Angie Bikou
  \brief    kfs-lite tools

  TODO/FIXME :
    1. General :
        - better error handling -> check return values, inputs, etc
        - make coding style more consistent
        - check for memory leaks
        - document functions
        - update usage
        - add kfs_ at the beginning of function's names ?
    2. Tree :
        - add the option to view the content of the files since the function exists (kfs_files)
    3. Build :
        - some of the constraints are not too difficult to fix (cf build function comment)
        - add a -d option for deleting files
        - add a -x option for a kernel to be put in /boot/kdata.x and /boot/kcode.x
            --> should only be allowed if /boot does not exist in the disk already ?
        - specification : each user should have its own dir in /home ?
            --> add a -u option to specify user whose files are added
        - specification : add -r option to replace ktools DirName argument or keep it the way it is?
    4. Split :
        - check the size condition in the "write loop", not sure it is correct at the limits
        - some of the constraints are not too difficult to fix (cf split function comment)
        - specification : add -r option to replace ktools DirName argument or keep it the way it is?
        - specification : should DirName exist before calling ktools ? (as of now, yes it should)

\*------------------------------------------------------------------------------------------------*/

#define LINUX
#define _XOPEN_SOURCE 500

#ifndef __DEPEND__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <ftw.h>
#include <sys/stat.h>
#endif

#include <kfs.h>
char *strdup(const char *s);

//--------------------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------------------

enum {
    CMD_DUMMY,
    CMD_TREE,
    CMD_BUILD,
    CMD_SPLIT
};
char **Argv;            //< array arguments
int Argc;               //< number of arguments
int Verbose;            //< from 0 to 2
int Command;            //< CMD_x
char * MbrFileName;     //< MBR filename
char * BootFileName;    //< Bootloader filename
char * DirName;         //< MBR file filename
char * KfsDiskName;     //< Kfs Disk
char * OptCPathname;
char * OptCNewFile;
int OptF = 0;

static char buffer[1<<12];

//--------------------------------------------------------------------------------------------------
// Utililties
//--------------------------------------------------------------------------------------------------

/*
* Returns a pointer to a static variable holding the absolute pathname of the last dentry
*   used to call this function.
* The depth of the dentry from the root should not be higher than 8.
*/
char* kfs_absolute_pathname(int dentry)
{
    static char path[256];
    memset(path, '\0', 256);
    char* current_name;
    int current_name_length;
    int current_dentry = dentry;
    int i = 0;

    /* If dentry is the root, put '/' in path and return */
    if (!current_dentry)
    {
        path[0] = '/';
        return path;
    }

    /* If not, for each dentry, add its name at the beginning of the path string */
    while(current_dentry)
    {
        /* Assuming dentry names max len is 28, no more than 8 iterations for a path of len 256 */
        if (i >= 8) return NULL;

        /* Get the name of the current dentry */
        current_name = kfs_name(current_dentry);

        /* Move the path to be able to copy the current name at the beginning of the path */
        current_name_length = strlen(current_name);
        memmove(path + current_name_length, path, strlen(path));
        strncpy(path, current_name, current_name_length);

        /* Move the path again to add '/' at the very beginning */
        memmove(path + 1, path, strlen(path));
        path[0] = '/';

        /* In the next iteration, the dentry will have to be the root of the current one */
        current_dentry = kfs_root(current_dentry);

        /* Increment i which ensures that the absolute pathname cannot be longer than a given len */
        i++;
    }

    /* Add the null terminating character */
    path[strlen(path)] = '\0';

    return path;
}

//--------------------------------------------------------------------------------------------------
// Tree
//--------------------------------------------------------------------------------------------------

void kfs_print_dentry (int dentry, int depth, int position)
{
    char buffer[256];                                       // formated filename
    int l;                                                  // directory level 0=root
    char types[4] = {'d','-','p','l'};                      // type name
    char * owners[] = {"k ","u1","u2","u3"};                // owner's name
    char * name = kfs_name (dentry);                        // get entry name
    int inode = kfs_inode(dentry);                          // get the associated inode
    buffer[0]=0;                                            // clear the buffer
    for (l = depth; l; l--) strcat (buffer, "  ");          // indent the name
    strcat (buffer, name);                                  // put the name
    strcat (buffer, (kfs_type(inode)==KFS_DIR)?"/":" ");    // add  / for a diectory
    for (l = strlen(buffer); l<30; l++) buffer[l]=' ';      // complete until 30 character
    buffer[l]=0;                                            // end the buffer
    printf ("d%-2x>i%-2x "                                  // dentry -> inode
            "%c%c%c%c%c%c%c %3d %s %7d %d "                 // inode data
            "<%-2xv%-2x>%-2x %s",                           // dentry tree structure
            dentry, inode,                                  //
            types[kfs_type(inode)],                         // inode type
            (kfs_mode(inode) & 0b100000)  ? 'r' : '-',      // inode permission mode
            (kfs_mode(inode) & 0b010000)  ? 'w' : '-',
            (kfs_mode(inode) & 0b001000)  ? 'x' : '-',
            (kfs_mode(inode) & 0b000100)  ? 'r' : '-',
            (kfs_mode(inode) & 0b000010)  ? 'w' : '-',
            (kfs_mode(inode) & 0b000001)  ? 'x' : '-',
            kfs_count(inode),                               // inode references counter
            owners[kfs_owner(inode)],                       // inode owner
            kfs_size(inode),                                // inode size
            kfs_mtime(inode),                               // inode relative modification time
            kfs_root(dentry),                               // dentry structure
            kfs_next(dentry),
            kfs_leaf(dentry),
            buffer);                                        // entry name
    int offset = 0;                                         // offset in file
    int page = kfs_page (inode, offset++);                  // first page number (-1 if ended)
    while (page >= 0) {                                     // while there are pages
        printf ("%3d", page);                               // print page number in disk
        page = kfs_page (inode, offset++);                  // find the next
    }
    printf ("\n");                                          // at last change the line
}

void kfs_tree (char *name)
{
    kfs_tree_cb (kfs_open(name), kfs_print_dentry);         // call kfs_print_dentry for all entry
}

void kfs_print_files(int dentry, int depth, int position)
{
    int inode = kfs_inode(dentry);
    int size = kfs_size(inode);
    int pg_offset = 0;

    printf("\n%s\n", kfs_absolute_pathname(dentry));

    if (size == 0) return;

    if (kfs_type(inode) == KFS_FILE) {
        while (pg_offset<<12 < size)
        {
            memset (buffer, 0, 1<<12);
            kfs_read(dentry, pg_offset, (void *) buffer);
            write(STDOUT_FILENO, buffer, 1<<12);
            pg_offset++;
        }
        printf("\n");
    }
}

void kfs_files(char* name)
{
    kfs_tree_cb(kfs_open(name), kfs_print_files);
}

//--------------------------------------------------------------------------------------------------
// Dummy disk
//--------------------------------------------------------------------------------------------------

char * mess (char * s)
{
    memset (buffer, 0, 1<<12);
    sprintf (buffer, "%s", s);
    return buffer;
}

void dummy_disk(void)
{
    int fd[32];
    char buffer[1<<12];

    fd[0] = kfs_open ("/bin/app.x");
    fd[1] = kfs_open ("/home");
    fd[2] = kfs_openat (fd[1], "angie");
    fd[3] = kfs_openat (fd[2], "lena1.pgm");
    fd[4] = kfs_openat (fd[1], "franck");
    fd[5] = kfs_openat (fd[1], "francois");
    fd[6] = kfs_link ("/home/angie/lena1.pgm", "/home/lena2.pgm");

    kfs_write (fd[3], 0, mess("Bonjour0"));
    kfs_set_size(fd[3], 8);

    kfs_write (fd[3], 3,  mess("Bonjour3"));
    kfs_set_size(fd[3], (3<<12) + 8);

    kfs_write (fd[3], 13, mess("Bonjour13"));
    kfs_set_size(fd[3], (13<<12) + 9);

    kfs_write (fd[3], 27, mess("Bonjour27"));
    kfs_set_size(fd[3], (27<<12) + 9);

    kfs_write (fd[3], 28, mess("Bonjour28"));
    kfs_set_size(fd[3], (28<<12) + 9);

    kfs_write (fd[3], 29, mess("Bonjour29"));
    kfs_set_size(fd[3], (29<<12) + 9);

    kfs_write (fd[3], 30, mess("Bonjour30"));
    kfs_set_size(fd[3], (30<<12) + 9);

    kfs_read ( fd[3],  0, (void *)buffer); //fprintf (stderr, "%d:%s\n", 0, buffer);
    kfs_read ( fd[3],  3, (void *)buffer); //fprintf (stderr, "%d:%s\n", 3, buffer);
    kfs_read ( fd[3], 13, (void *)buffer); //fprintf (stderr, "%d:%s\n", 13, buffer);
    kfs_read ( fd[3], 27, (void *)buffer); //fprintf (stderr, "%d:%s\n", 27, buffer);
    kfs_read ( fd[3], 28, (void *)buffer); //fprintf (stderr, "%d:%s\n", 28, buffer);
    kfs_read ( fd[3], 29, (void *)buffer); //fprintf (stderr, "%d:%s\n", 29, buffer);
    kfs_read ( fd[3], 30, (void *)buffer); //fprintf (stderr, "%d:%s\n", 30, buffer);

    kfs_tree ("/");
}

//--------------------------------------------------------------------------------------------------
// Build command
//--------------------------------------------------------------------------------------------------

void add_new_file(const char *host_file, int kfs_dentry)
{
    int page_offset = 0;
    int nb_bytes_read = 0;

    /* Try to open the host's file */
    int fd = open(host_file, O_RDONLY);

    /* Prepare buffer to receive the first page of the file */
    memset (buffer, 0, 1<<12);

    /* Read the file, in chunks of 4kib, and write its content to the kfs pathname via the buffer */
    while ((nb_bytes_read = read(fd, buffer, 1<<12)))
    {
        kfs_write(kfs_dentry, page_offset, buffer);
        kfs_set_size(kfs_dentry, nb_bytes_read + (page_offset<<12));
        page_offset++;
        memset (buffer, 0, 1<<12);
    }

    /* Close the file descriptor of the host's file */
    close(fd);
}

/*
Callback function for nftw (nftw = glibc "new file tree walk" function).
- Builds a kfs disk from a linux directory given as a relative path
(--> The possibility of giving absolute paths could be implemented but may be a bit tedious...
on 1st call, check whether fpath starts with '/' then use ftwbuf->base to remember
where the relevant part of fpath starts)
- Symbolic links are followed and treated as regular files
(--> to change this, set FTW_PHYS flag when calling nftw...).
- Hard links are written in the kfs disk as different files, with different inodes
(--> to change this, check sb->st_nlink before writing a file in the kfs disk...)
- Empty directories are added as dentries with the type KFS_FILE
- Permissions are not taken into consideration
- Kfs owner is "kernel" for every file
*/
int build(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    /* 1st call : if the pathname is not a directory that can be read, return 1 to stop the walk */
    if ((ftwbuf->level == 0) && (typeflag != FTW_D))
    {
        printf("%s is not a directory that can be read\n", fpath);
        return 1;
    }

    /* 1st call : if the pathname is a valid directory the walk can continue */
    if ((ftwbuf->level == 0) && (typeflag == FTW_D))
    {
        return 0;
    }

    /* Subsequent calls : start with creating a new dentry in kfs  */
    char* new_dentry_name = malloc(strlen(fpath) + 2);
    sprintf(new_dentry_name, "/%s", fpath);
    int kfs_dentry = kfs_open(new_dentry_name);
    free(new_dentry_name);

    /* If it is a regular file, write its content to kfs */
    if (typeflag == FTW_F)
    {
        add_new_file(fpath, kfs_dentry);

        return 0;
    }

    /* If it is a directory, there isn't anything more to do */
    if (typeflag == FTW_D)
    {
        /* In kfs, set the dentry type to directory ??? */

        return 0;
    }

    /* If it is not any of the above, return an error to stop the walk */
    return 2;
}

//--------------------------------------------------------------------------------------------------
// Split command
//--------------------------------------------------------------------------------------------------

/*
Callback function for kfs_tree.
Splits a kfs disk in a linux directory
- The linux directory has to exist and be empty before calling split
- Symbolic links are ignored
- Pipes are ignored
- Hard links are written as different files with different inodes
- Permissions on host : directories = rwx, files = rw.
*/
void split (int dentry, int depth, int position)
{
    /* If this is the root, nothing needs to be done */
    if (!dentry) return;

    /* Get inode information about this dentry and create the string for the new pathname on host */
    int inode = kfs_inode(dentry);
    int size = kfs_size(inode);
    char* absolute = kfs_absolute_pathname(dentry);
    char* new_path = malloc(strlen(absolute) + strlen(DirName) + 1);
    strncpy(new_path, DirName, strlen(DirName));
    strcat(new_path, absolute);

    printf("%s\n", new_path);

    /* If it is a directory, create it on host */
    if (kfs_type(inode) == KFS_DIR)
    {
        mkdir(new_path, 0777);
        free(new_path);
        return;
    }

    /* If it is a file, create it and write its content on host */
    if (kfs_type(inode) == KFS_FILE)
    {
        int fd = open(new_path, O_RDWR | O_CREAT | O_TRUNC, 0666);
        int page_offset = 0;
        /* Write every page but the last */
        while(((page_offset<<12) + (1<<12)) <= size)
        {
            memset(buffer, 0, 1<<12);
            kfs_read(dentry, page_offset, (void*) buffer);
            write(fd, buffer, 1<<12);
            page_offset++;
        }
        /* Write the last page */
        memset(buffer, 0, 1<<12);
        kfs_read(dentry, page_offset, (void*) buffer);
        write(fd, buffer, strlen(buffer));

        free(new_path);
        return;
    }
}

//--------------------------------------------------------------------------------------------------
// get options
//--------------------------------------------------------------------------------------------------

void usage (void)
{
    printf ("\nUsage : %s [-h] [-v level] [-m mbr] [-b boot] [-c pathname]\n", Argv[0]);
    printf ("                  <command> <kfsd> [dir]\n\n");
    printf ("         -h  this help\n");
    printf ("         -v  verbose mode level (0, 1, 2)\n");
    printf ("     -m mbr  mbr executable file\n");
    printf ("    -b boot  bootloader executable file\n");
    //printf (" -c pathname copy file pathname\n");
    printf ("    command  < tree | build | split | dummy >\n");
    printf ("       kfsd  kfs disk name (with .kfs extension) \n");
    printf ("        dir  Linux directory\n");
    printf ("\n");
    exit (1);
}

void getoption (int argc, char *argv[])
{
    char option;

    Argc = argc;
    Argv = argv;

    // optional arguments
    // ------------------------------------------------------------------
    while ((option = getopt (argc, argv, "hv:m:b:c:f")) != EOF) {
        switch (option) {
        case 'v':
            Verbose = atoi(optarg);
            if ((Verbose < 0) || (Verbose > 2))
                usage ();
            break;
        case 'm':
            MbrFileName = strdup (optarg);
            // FIXME verfier exension et type
            break;
        case 'b':
            BootFileName = strdup (optarg);
            // FIXME verfier exension et type
            break;
#if 0
        case 'c':
            /* FIXME Ajouter les vérifications */
            char * optCArg = strdup(optarg);

            OptCPathname = strdup(strtok(optCArg, ":"));
            OptCNewFile = strtok(NULL, "");

            if (OptCNewFile)
                OptCNewFile = strdup(OptCNewFile);

            break;
#endif
        case 'f':
            OptF = 1;
            break;
        default:
            usage ();
        }
    }

    // mandatory arguments
    // ------------------------------------------------------------------
    if (optind < argc) {
        if (strcmp (argv[optind], "dummy")==0)  Command = CMD_DUMMY; else
        if (strcmp (argv[optind], "build")==0)  Command = CMD_BUILD; else
        if (strcmp (argv[optind], "split")==0)  Command = CMD_SPLIT; else
        if (strcmp (argv[optind], "tree")==0)   Command = CMD_TREE;  else
        usage();
        optind++;
    } else usage();

    if (optind < argc) {
        // FIXME verfier exension et type
        KfsDiskName = strdup (argv[optind]);
        optind++;
    } else usage();

    // optional arguments that depend on command
    // ------------------------------------------------------------------
    /* if a DirName needs to be retrieved */
    if ((Command != CMD_DUMMY)&&(Command != CMD_TREE) \
        &&(!OptCPathname)&&(!MbrFileName)&&(!BootFileName)) {
        if (optind < argc) {
            DirName = strdup (argv[optind]);
            // FIXME verfier exension et type
            optind++;
        }
        else usage();
    }

    /* There should not be any more arguments at this point */
    if (optind != argc)
        usage();
}

int main(int argc, char *argv[])
{
    getoption (argc, argv);

    switch (Command) {

        case CMD_DUMMY:
            dummy_disk();
            kfs_disk_save (KfsDiskName);
            break;

        case CMD_TREE:
            kfs_disk_load (KfsDiskName);
            if (OptF) {
                kfs_files("/");
            }
            else {
                kfs_tree("/");
            }
            break;

        case CMD_BUILD:
            /* Add an MBR if one was provided */
            if (MbrFileName)
            {
                if (!DirName) {
                    kfs_disk_load (KfsDiskName);
                }
                kfs_add_mbr(MbrFileName);
            }

            /* Add a kernel bootloader if one was provided */
            if (BootFileName)
            {
                if (!DirName) {
                    kfs_disk_load (KfsDiskName);
                }
                kfs_add_vbr(BootFileName);
            }

            /* Build without any options = build from linux directory */
            if (!OptCPathname) {

                /* Walk through the linux directory and write its content in kfs */
                if (DirName) nftw(DirName, build, 20, 0);
            }

            /* Build with c option = add a file to a pathname on an existing disk */
            if (OptCPathname)
            {
                /* Charge the disk to modify in kfs (if it wasn't charged before) */
                if (!(MbrFileName || BootFileName)) {
                    if (kfs_disk_load (KfsDiskName) <= 0) {
                        fprintf (stderr, "Could not load %s\n", KfsDiskName);
                        exit(1);
                    }
                }

                /* Create (or open) the pathname in kfs */
                int kfs_dentry = kfs_open(OptCPathname);

                /* If the user provided a filename from the host, try to write its content in kfs */
                if (OptCNewFile)
                {
                    add_new_file(OptCNewFile, kfs_dentry);
                }
            }

            /* Lastly, save the new (or modified) kfs disk on host */
            kfs_disk_save(KfsDiskName);

            break;

        case CMD_SPLIT:
            /* First, load the kfs disk from host */
            kfs_disk_load (KfsDiskName);

            /* Create the host directory where the kfs disk will be split */
            mkdir(DirName, 0777);

            /* Now we can walk through this kfs disk and write its content to the host directory */
            kfs_tree_cb(0, split);

            break;
    }
    return 0;
}
