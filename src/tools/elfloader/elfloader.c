/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-27
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     tools/elfloader/elfloader.c
  \author   Franck Wajsburt
  \brief    section loader for an executable
            
     ┌────────────┐         Header information for a 32-bit ELF file, including the file type, 
  ┌──┼ elf32_Ehdr │         machine type, entry point address, and offsets in the section headers.
  │  └────────────┘         
  │  ┌────────────┐◄───┐    Each section is raw content stored as a continuous array of bytes.   
  │  │    text    │    │    For the ko6 loader, the loadable sections are .text, .data, and .bss.
  │  ├────────────┤◄─┐ │    The .text section contains binary code, the .data section contains 
  │  │    data    │  │ │    initialized global variables, and the .bss section reserves space 
  │  │            │  │ │    for uninitialized global variables. 
  │  └────────────┘  │ │ 
  └─►┌────────────┐  │ │    Section header: array of Elf32_Shdr structures describes all sections
     │ Elf32_Shdr │──┘ │    of a 32-bit ELF file, with each entry giving the properties 
     │            │────┘    (name, type, address, size, etc.) of one section
     └────────────┘                                                           https://asciiflow.com
\*------------------------------------------------------------------------------------------------*/

#include <elfloader.h>

elf_t *elf_open (const char *filename, const char *section_names[], int section_count)
{
    RETURN (NULL, section_count > MAX_SECTIONS, "Too many sections");

    int fd = OPENR (filename);                                              // open elf file
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
    PRINT ("File size : %d\n", strtab.sh_offset + strtab.sh_size);

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

int elf_load_section (elf_t *elf, int section_index, char * output_filename) 
{
    RETURN (-1, section_index >= MAX_SECTIONS, "Invalid section index");

    int size      = elf->sections[section_index].header.sh_size;
    int offset    = elf->sections[section_index].header.sh_offset;
    char * name   = elf->sections[section_index].name;
    unsigned addr = elf->sections[section_index].addr;
    void *data    = MALLOC (size); 
    RETURN (-1, !data, "malloc data section");

    elf->sections[section_index].data = data; 
    lseek (elf->fd, offset, SEEK_SET);
    read (elf->fd, data, size);

    int out_fd = OPENW (output_filename);
    RETURN (-1, out_fd < 0, "not loaded section");
    write (out_fd, data, size);
    close (out_fd);

    PRINT ("Section %s in %s (%d bytes) addr=%08x\n", name, output_filename, size, addr);
    return 0;
}
