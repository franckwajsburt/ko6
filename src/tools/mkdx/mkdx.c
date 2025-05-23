/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-03-14
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     tools/mkdx.c
  \author   Franck Wajsburt
  \brief    build a simple disk image with a single directory

  0   1   2   3   4   5   6   7   8   9  ... LBA (1 block = 4 kB)
  ┌───┌───────────┌───────┌───────────────┐
  │DIR│   app1.x  │app2.x │     app3.x    │  disk image built
  └───└───────────└───────└───────────────┘
      ┌─────────────┐
  DIR:│app1.x 1 11kB│ name[24],LBA,size
      │app2.x 4 7kB │
      │app3.x 6 15kB│
      │             │ 128 file descr.
      └─────────────┘                                                        https://asciiflow.com
\*------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#define PAGE_SIZE 4096
#define MAX_FILES 128

typedef struct {
    char     name[24];   // filename 23 bytes + '\0'
    uint32_t lba;        // logical block position
    uint32_t size;       // size if bytes
} entry_t;

entry_t Dir[MAX_FILES];
int     Nb_file = 1;    // file n°0 is not used
int     Disk_fd;

void usage (const char *s)
{
    if (errno) perror (s);
    else fprintf (stderr, "Error: %s\n", s);
    fprintf (stderr, "Usage: mkdx <diskname> <file1> <file2> ...\n");
    exit (1);
}

void copy_file_to_disk (char *pathname, int file_index, int *current_lba)
{
    int in_fd = open (pathname, O_RDONLY);
    if (in_fd < 0) usage (pathname);

    char *name;
    for (name = pathname + strlen (pathname); (name != pathname) && (*name != '/'); name--);
    if (*name == '/') name++;

    struct stat st;
    fstat (in_fd, &st);

    strncpy (Dir[file_index].name, name, 23);
    Dir[file_index].name[23] = '\0';
    Dir[file_index].lba = *current_lba;
    Dir[file_index].size = st.st_size;

    char buffer[PAGE_SIZE];
    int bytes_read;
    while ((bytes_read = read (in_fd, buffer, PAGE_SIZE)) > 0) {
        write (Disk_fd, buffer, bytes_read);
        (*current_lba) += (bytes_read + PAGE_SIZE - 1) / PAGE_SIZE;  // block alignment
    }

    close (in_fd);
}

int main (int argc, char *argv[])
{
    if (argc < 3) usage ("Not enough arguments");

    Disk_fd = open (argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (Disk_fd < 0) usage (argv[1]);

    lseek (Disk_fd, PAGE_SIZE, SEEK_SET);                           // 1 block is for directory
    int current_lba = 1;                                            // first block for file

    for (int i = 2; i < argc && Nb_file < MAX_FILES; i++, Nb_file++) {
        copy_file_to_disk (argv[i], Nb_file, &current_lba);
    }

    lseek (Disk_fd, 0, SEEK_SET);                                   // write de directory
    write (Disk_fd, Dir, sizeof(Dir));
    close (Disk_fd);

    printf ("Done %d files written to disk image '%s'\n", Nb_file, argv[1]);
    for (int i = 1; (i < MAX_FILES) && (Dir[i].name[0]) ; i++) {
        printf ("%24s ; index %3d ; lba %4d ; size %d\n", Dir[i].name, i, Dir[i].lba, Dir[i].size);
    }
    return 0;
}
