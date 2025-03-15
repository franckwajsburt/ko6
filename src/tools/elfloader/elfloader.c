/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-03-14
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     kernel/kinit.c
  \author   Franck Wajsburt
  \brief    section loader for an executable
            
\*------------------------------------------------------------------------------------------------*/

#ifdef _HOST_
#   include <stdio.h>
#   include <stdlib.h>
#   include <fcntl.h>
#   include <unistd.h>
#   include <string.h>
#   include <sys/types.h>
#   include <stdint.h>
#   define MALLOC(s) malloc(s)
#   define FREE(s) free(s)
#   define P(fmt,var) fprintf(stderr, #var" : "fmt, var);
#else
#   define MALLOC(s) kmalloc(s)
#   define FREE(s) kfree(s)
#   define P(fmt,var) 
#endif

#include <elfloader.h>

#define RETURN(e,c,m,...) if(c){fprintf(stderr,"Error "m"\n");__VA_ARGS__;return e;}

elf_t *elf_open (const char *filename, const char *section_names[], int section_count)
{
    RETURN (NULL, section_count > MAX_SECTIONS, "Too many sections");

    int fd = open (filename, O_RDONLY);                                     // open elf file
    RETURN (NULL, fd < 0, "open");

    elf_t *elf = MALLOC (sizeof(elf_t));                                    // malloc elf struct
    RETURN (NULL, elf == NULL, "malloc", close (fd));
    elf->fd = fd;
    elf->section_count = 0;

    int s = read (fd, &elf->header, sizeof(Elf32_Ehdr));                    // read header
    RETURN (NULL, s != sizeof(Elf32_Ehdr), "ELF header", FREE(elf), close(fd));

    int r = memcmp (elf->header.e_ident, ELFMAGIC, 4);                      // magic value
    RETURN (NULL, r != 0, "Not a valid ELF file!", FREE (elf), close(fd));

    int m = elf->header.e_machine;                                          // Architecture Type
    RETURN (NULL, m != EM_MIPS, "Not MIPS Exec", FREE (elf), close(fd));
   
    lseek (fd, elf->header.e_shoff, SEEK_SET);                              // read section table
    Elf32_Shdr sections[elf->header.e_shnum];
    read (fd, sections, sizeof(Elf32_Shdr) * elf->header.e_shnum);
    P("%04x\n",elf->header.e_shnum);

    Elf32_Shdr strtab = sections[elf->header.e_shstrndx];                   // read section's name
    char *section_name_table = MALLOC (strtab.sh_size);
    RETURN (NULL, !section_name_table, "malloc section_name_table", FREE(elf), close(fd));
    lseek (fd, strtab.sh_offset, SEEK_SET);
    read (fd, section_name_table, strtab.sh_size);

    for (int i = 0; i < elf->header.e_shnum; i++) {                         // find sections
        const char *name = section_name_table + sections[i].sh_name;
        if (section_names) {
            for (int j = 0; j < section_count; j++) {
                if (strcmp (name, section_names[j]) == 0) {
                    int index = elf->section_count++;
                    strncpy (elf->sections[index].name, name, 15);
                    elf->sections[index].header = sections[i];
                    elf->sections[index].data = NULL;                       // not yet loaded  
                    elf->sections[index].addr = sections[i].sh_addr;        // address in memory
                    P("%-16s ",name);
                    P("%08x ",sections[i].sh_addr);
                    P("%04x\n",sections[i].sh_type);
                }
            }
        } 
        else if ((sections[i].sh_addr)&&(sections[i].sh_type == SHT_PROGBITS)) {
            int index = elf->section_count++;
            strncpy (elf->sections[index].name, name, 15);
            elf->sections[index].header = sections[i];
            elf->sections[index].data = NULL;                       // not yet loaded  
            elf->sections[index].addr = sections[i].sh_addr;        // address in memory
            P("PROGBITS %s\n",name);
        } 
        else if (sections[i].sh_type == SHT_NOBITS) {
            int index = elf->section_count++;
            strncpy (elf->sections[index].name, name, 15);
            elf->sections[index].header = sections[i];
            elf->sections[index].data = NULL;                       // not yet loaded  
            elf->sections[index].addr = sections[i].sh_addr;        // address in memory
            P("NOBITS %s\n",name);
        }
    }

    FREE (section_name_table);
    return elf;
}

void elf_close (elf_t *elf) 
{
    if (!elf) return;

    for (int i = 0; i < MAX_SECTIONS; i++) {
        if (elf->sections[i].data) FREE (elf->sections[i].data);
    }
    close (elf->fd);
    FREE (elf);
}

int elf_load_section (elf_t *elf, int section_index) 
{
    RETURN (-1, section_index >= MAX_SECTIONS, "Invalid section index");

    int size    = elf->sections[section_index].header.sh_size;
    int offset  = elf->sections[section_index].header.sh_offset;
    void *data  = MALLOC (size); 
    RETURN (-1, !data, "malloc data section");

    elf->sections[section_index].data = data; 
    lseek (elf->fd, offset, SEEK_SET);
    read (elf->fd, data, size);

    return 0;
}

int elf_dump_section (elf_t *elf, int section_index, const char *output_filename) 
{
    RETURN (-1, section_index >= MAX_SECTIONS, "Invalid section index");

    int size      = elf->sections[section_index].header.sh_size;
    char * name   = elf->sections[section_index].name;
    void * data   = elf->sections[section_index].data;
    unsigned addr = elf->sections[section_index].addr;
    RETURN (-1, !data, "not loaded section");

    int out_fd = open (output_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    RETURN (-1, out_fd < 0, "not loaded section");

    write (out_fd, data, size);
    close (out_fd);

    printf ("Section %s in %s (%d bytes) addr=%08x\n", 
            name, output_filename, size, addr);
    return 0;
}

int main (int argc, char **argv) 
{
    RETURN (1, argc < 2, "Usage: elf_loader <ELF file> [section...]");
    
    elf_t *elf = (argc == 2) ? elf_open (argv[1], (const char **)&argv[2], argc-2)
                             : elf_open (argv[1], NULL, 0);
    RETURN (1, !elf, "Section not found");

    for (int i = 0; i < elf->section_count; i++) {
        char filename[32];
        strncpy (filename, elf->sections[i].name, sizeof(filename)-1);
        strncat (filename, ".bin", sizeof(filename)-1);
        elf_load_section (elf, i);
        elf_dump_section (elf, i, filename);
    }

    elf_close (elf);
    return 0;
}
