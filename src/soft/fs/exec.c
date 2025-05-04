/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-27
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     fs/exec.c
  \author   Franck Wajsburt
  \brief    program loader and process starter
            
     ┌────────────┐         Header information for a 32-bit ELF file, including the file type, 
  ┌──┼ elf32_Ehdr │         machine type, entry point address, and offsets to the program and
  │  └────────────┘         section headers. 
  │  ┌────────────┐◄───┐    
  │  │    text    │    │    Each section is raw content stored as a continuous array of bytes.    
  │  ├────────────┤◄─┐ │    For the ko6 loader, the loadable sections are .text, .data, and .bss.
  │  │    data    │  │ │    The .text section contains binary code, the .data section contains 
  │  │            │  │ │    initialized global variables, and the .bss section reserves space 
  │  └────────────┘  │ │    for uninitialized global variables. 
  └─►┌────────────┐  │ │                                                                          
     │ Elf32_Shdr │──┘ │    Section header: array of Elf32_Shdr structures describes all sections
     ├────────────┤    │    of a 32-bit ELF file, with each entry giving the properties 
     │            │────┘    (name, type, address, size, etc.) of one section 
     └────────────┘                                                          https://asciiflow.com 
\*------------------------------------------------------------------------------------------------*/

#include <elfloader.h>

#define MALLOC(s) kmalloc(s)
#define FREE(s) kfree(s)
#define RETURN(e,c,m,...) if(c){kprintf("Error "m"\n");__VA_ARGS__;return -e;}
#define OPENR(f) open (f)
#define OPENW(f)
#define PRINT(fmt,...)
#define MAX_SECTIONS 8              ///< Maximum number of sections that can be loaded.

/**
 * Some defines
 */
#define ELFMAGIC     "\x7F""ELF"    ///< ELF MAGIC number
#define EM_MIPS      8              ///< Architecture Type
#define SHT_PROGBITS 1              ///< Section type : Program-defined contents
#define SHT_NOBITS   8              ///< Section type : Data occupies no space in the file

/**
 * \brief ELF Header Structure (32-bit ELF)
 */
typedef struct {
    unsigned char  e_ident[16];     ///< ELF identification bytes
    unsigned short e_type;          ///< Object file type
    unsigned short e_machine;       ///< Architecture
    unsigned int   e_version;       ///< Object file version
    unsigned int   e_entry;         ///< Entry point virtual address
    unsigned int   e_phoff;         ///< Program header table file offset
    unsigned int   e_shoff;         ///< Section header table file offset
    unsigned int   e_flags;         ///< Processor-specific flags
    unsigned short e_ehsize;        ///< ELF header size in bytes
    unsigned short e_phentsize;     ///< Program header table entry size
    unsigned short e_phnum;         ///< Number of program header entries
    unsigned short e_shentsize;     ///< Section header table entry size
    unsigned short e_shnum;         ///< Number of section header entries
    unsigned short e_shstrndx;      ///< Section header string table index
} Elf32_Ehdr;

/**
 * \brief ELF Section Header Structure (32-bit ELF)
 */
typedef struct {
    unsigned int   sh_name;         ///< Section name (offset in string table)
    unsigned int   sh_type;         ///< Section type
    unsigned int   sh_flags;        ///< Section attributes
    unsigned int   sh_addr;         ///< Virtual address in memory
    unsigned int   sh_offset;       ///< Offset in file
    unsigned int   sh_size;         ///< Size of section
    unsigned int   sh_link;         ///< Link to another section
    unsigned int   sh_info;         ///< Additional section information
    unsigned int   sh_addralign;    ///< Section alignment
    unsigned int   sh_entsize;      ///< Entry size if section holds a table
} Elf32_Shdr;

/**
 * \brief Loads an ELF binary into memory.
 *        This function opens the ELF file, reads the header, loads all
 *        relevant sections (PROGBITS and NOBITS) into memory at the specified addresses,
 *        and sets the process entry point.
 * \param path  Path to the ELF executable.
 * \param proc  Pointer to the process structure to initialize (entry point field).
 * \return 0 on success, negative error code on failure.
 */
static int load_elf(const char *path, proc_t *proc)
{
    int fd = open(path);                                    // (1) Open the ELF file
    if (fd < 0) {
        return -ENOENT;
    }
    Elf32_Ehdr ehdr;                                        // (2) Read the ELF header
    if (read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr)) {
        close(fd);
        return -EINVAL;
    }
    if (memcmp(ehdr.e_ident, ELFMAGIC, 4) != 0) {           // (3) Check ELF magic number
        close(fd);
        return -EINVAL;
    }
    if (ehdr.e_machine != EM_MIPS) {                        // (4) Check arch. (MIPS expected)
        close(fd);
        return -EINVAL;
    }
    lseek(fd, ehdr.e_shoff, SEEK_SET);                      // (5) Read section headers
    Elf32_Shdr sections[ehdr.e_shnum];
    if (read(fd, sections, sizeof(Elf32_Shdr)*ehdr.e_shnum) 
    != sizeof(Elf32_Shdr)*ehdr.e_shnum) {
        close(fd);
        return -EINVAL;
    }
    for (int i = 0; i < ehdr.e_shnum; i++) {                // (6) Load sections into memory
        Elf32_Shdr *sh = &sections[i];
        if (sh->sh_flags & 2) {                             // SHF_ALLOC (section must be loaded)
            void *buf = (void *)(unsigned long)sh->sh_addr;
            if (sh->sh_type == SHT_PROGBITS) {
                lseek(fd, sh->sh_offset, SEEK_SET);         // Load data from file into memory
                if (read(fd, buf, sh->sh_size) != sh->sh_size) {
                    close(fd);
                    return -EIO;
                }
            }
            else if (sh->sh_type == SHT_NOBITS) {           // Zero-fill memory for .bss
                memset(buf, 0, sh->sh_size);
            }
        }
    }
    proc->entry_point = ehdr.e_entry;                       // (7) Set the process entry point
    close(fd);                                              // (8) Close the ELF file
    return 0;
}
