#ifndef ELF32_LOADER_H
#define ELF32_LOADER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    uint8_t *data;          // Dynamically allocated buffer containing .text bytes
    size_t size;            // Length in bytes of the .text section
    uint32_t vaddr;         // Virtual address where .text is loaded in memory
    uint32_t entry_point;   // ELF entry point (e_entry)
} elf32_section_t;

// Loads the .text section of a 32-bit ELF executable.
// Returns 0 on success, or -1 on error (with error printed to stderr).
int elf32_load_text_section(const char *filepath, elf32_section_t *sec);

// Frees the memory allocated for the section.
void elf32_free_section(elf32_section_t *sec);

#endif // ELF32_LOADER_H
