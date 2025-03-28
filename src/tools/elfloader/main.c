/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-03-14
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     tools/elfloader.c
  \author   Franck Wajsburt
  \brief    section loader for an executable
            
\*------------------------------------------------------------------------------------------------*/

#include <elfloader.h>

int main (int argc, char **argv) 
{
    RETURN (1, argc < 2, "Usage: elf_loader <ELF file> [section...]");
    
    elf_t *elf = (argc == 2) ? elf_open (argv[1], (const char **)&argv[2], argc-2)
                             : elf_open (argv[1], NULL, 0);
    RETURN (1, !elf, "Section not found");

    for (int i = 0; i < elf->section_count; i++) {
        char filename[32];
        char * secname = elf->sections[i].name; 
        strncpy (filename, secname + (*secname == '.'), sizeof(filename)-1);
        strncat (filename, ".bin", sizeof(filename)-1);
        elf_load_section (elf, i, filename);
    }

    elf_close (elf);
    return 0;
}
