/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-03-14
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     kernel/kinit.c
  \author   Franck Wajsburt
  \brief    section loader for an executable

\*------------------------------------------------------------------------------------------------*/

#ifndef _ELFLOADER_H_
#define _ELFLOADER_H_

#include <elfminimal.h>

#define MAX_SECTIONS 8                  ///< Maximum number of sections that can be loaded.

/**
 * \struct elf_t
 * \brief Represents an ELF file and its selected sections.
 */
typedef struct {
    int fd;                             ///< File descriptor of the opened ELF file.
    Elf32_Ehdr header;                  ///< ELF file header.
    int section_count;                  ///< Number of sections successfully loaded.
    struct {                            ///< Represents a loaded ELF section.
        char name[16];                  ///< Name of the ELF section.
        Elf32_Shdr header;              ///< ELF section header.
        void *data;                     ///< Memory address where the section is loaded.
        unsigned addr;                  ///< 
    } sections[MAX_SECTIONS];           ///< Array of loaded sections (up to MAX_SECTIONS).
} elf_t;

/**
 * \brief Opens an ELF file and loads metadata for the specified sections.
 *        This function reads the ELF header and section table to identify and retrieve
 *        metadata for the requested sections. It does not load the actual section data into memory.
 * \param filename The name of the ELF file to open.
 * \param section_names An array of section names to search for.
 *        if NULL, open all PROGBITS and NOBITS sections
 * \param section_count The number of sections to load.
 * \return A pointer to an `elf_t` structure containing metadata for the requested sections,
 *         or `NULL` in case of an error (e.g., file not found, invalid ELF format).
 */
elf_t *elf_open(const char *filename, const char *section_names[], int section_count);

/**
 * \brief Closes an ELF file and frees allocated memory.
 *        This function closes the ELF file and releases all allocated memory,
 *        including section metadata and loaded data.
 * \param elf Pointer to an `elf_t` structure representing the opened ELF file.
 */
void elf_close(elf_t *elf);

/**
 * \brief Loads an ELF section's content into memory.
 *        This function loads the binary content of a previously identified ELF section
 *        into memory, making it available for further processing.
 * \param elf Pointer to an `elf_t` structure containing ELF metadata.
 * \param section_index Index of the section to load (must be less than `section_count`).
 * \return 0 on success, -1 on error (e.g., invalid section index, memory allocation failure).
 */
int elf_load_section(elf_t *elf, int section_index);

/**
 * \brief Writes a loaded ELF section to a binary file.
 *        This function saves the content of a previously loaded ELF section to a binary file,
 *        allowing it to be analyzed or extracted.
 * \param elf Pointer to an `elf_t` structure containing the loaded sections.
 * \param section_index Index of the section to write (must be less than `section_count`).
 * \param output_filename The name of the output file where the section will be saved.
 * \return 0 on success, -1 if an error occurs (e.g., section not loaded, write error).
 */
int elf_dump_section(elf_t *elf, int section_index, const char *output_filename);

#endif // _ELFLOADER_H_

