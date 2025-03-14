#ifndef ELF_H_MINIMAL
#define ELF_H_MINIMAL

#include <stdint.h>

/**
 * \brief ELF Header Structure (32-bit ELF)
 */
typedef struct {
    unsigned char e_ident[16]; ///< ELF identification bytes
    uint16_t e_type;           ///< Object file type
    uint16_t e_machine;        ///< Architecture
    uint32_t e_version;        ///< Object file version
    uint32_t e_entry;          ///< Entry point virtual address
    uint32_t e_phoff;          ///< Program header table file offset
    uint32_t e_shoff;          ///< Section header table file offset
    uint32_t e_flags;          ///< Processor-specific flags
    uint16_t e_ehsize;         ///< ELF header size in bytes
    uint16_t e_phentsize;      ///< Program header table entry size
    uint16_t e_phnum;          ///< Number of program header entries
    uint16_t e_shentsize;      ///< Section header table entry size
    uint16_t e_shnum;          ///< Number of section header entries
    uint16_t e_shstrndx;       ///< Section header string table index
} Elf32_Ehdr;

/**
 * \brief ELF Program Header Structure (32-bit ELF)
 */
typedef struct {
    uint32_t p_type;           ///< Segment type
    uint32_t p_offset;         ///< Offset in file
    uint32_t p_vaddr;          ///< Virtual address in memory
    uint32_t p_paddr;          ///< Physical address (unused in many cases)
    uint32_t p_filesz;         ///< Size of segment in file
    uint32_t p_memsz;          ///< Size of segment in memory
    uint32_t p_flags;          ///< Segment flags
    uint32_t p_align;          ///< Alignment of segment
} Elf32_Phdr;

/**
 * \brief ELF Section Header Structure (32-bit ELF)
 */
typedef struct {
    uint32_t sh_name;          ///< Section name (offset in string table)
    uint32_t sh_type;          ///< Section type
    uint32_t sh_flags;         ///< Section attributes
    uint32_t sh_addr;          ///< Virtual address in memory
    uint32_t sh_offset;        ///< Offset in file
    uint32_t sh_size;          ///< Size of section
    uint32_t sh_link;          ///< Link to another section
    uint32_t sh_info;          ///< Additional section information
    uint32_t sh_addralign;     ///< Section alignment
    uint32_t sh_entsize;       ///< Entry size if section holds a table
} Elf32_Shdr;

#endif // ELF_H_MINIMAL

