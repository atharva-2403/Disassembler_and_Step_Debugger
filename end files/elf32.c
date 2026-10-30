#include "elf32.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>

int elf32_load_text_section(const char *filepath, elf32_section_t *sec) {
    if (!filepath || !sec) {
        return -1;
    }

    memset(sec, 0, sizeof(*sec));

    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        perror("[ELF32 ERROR] open");
        return -1;
    }

    // Read ELF header
    Elf32_Ehdr ehdr;
    if (read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr)) {
        fprintf(stderr, "[ELF32 ERROR] Failed to read ELF header.\n");
        close(fd);
        return -1;
    }

    // Validate ELF magic bytes
    if (ehdr.e_ident[EI_MAG0] != ELFMAG0 ||
        ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
        ehdr.e_ident[EI_MAG3] != ELFMAG3) {
        fprintf(stderr, "[ELF32 ERROR] Not a valid ELF file.\n");
        close(fd);
        return -1;
    }

    // Ensure 32-bit ELF
    if (ehdr.e_ident[EI_CLASS] != ELFCLASS32) {
        fprintf(stderr, "[ELF32 ERROR] File is not a 32-bit ELF (EI_CLASS = %d).\n", ehdr.e_ident[EI_CLASS]);
        close(fd);
        return -1;
    }

    sec->entry_point = ehdr.e_entry;

    // Read Section Headers
    if (ehdr.e_shoff == 0 || ehdr.e_shnum == 0) {
        fprintf(stderr, "[ELF32 ERROR] No section headers found in ELF.\n");
        close(fd);
        return -1;
    }

    Elf32_Shdr *shdrs = malloc(sizeof(Elf32_Shdr) * ehdr.e_shnum);
    if (!shdrs) {
        perror("[ELF32 ERROR] malloc shdrs");
        close(fd);
        return -1;
    }

    if (lseek(fd, ehdr.e_shoff, SEEK_SET) < 0 ||
        read(fd, shdrs, sizeof(Elf32_Shdr) * ehdr.e_shnum) != (ssize_t)(sizeof(Elf32_Shdr) * ehdr.e_shnum)) {
        fprintf(stderr, "[ELF32 ERROR] Failed to read section headers.\n");
        free(shdrs);
        close(fd);
        return -1;
    }

    // Read Section Header String Table
    if (ehdr.e_shstrndx >= ehdr.e_shnum) {
        fprintf(stderr, "[ELF32 ERROR] Invalid e_shstrndx.\n");
        free(shdrs);
        close(fd);
        return -1;
    }

    Elf32_Shdr *shstrtab_hdr = &shdrs[ehdr.e_shstrndx];
    char *shstrtab = malloc(shstrtab_hdr->sh_size);
    if (!shstrtab) {
        perror("[ELF32 ERROR] malloc shstrtab");
        free(shdrs);
        close(fd);
        return -1;
    }

    if (lseek(fd, shstrtab_hdr->sh_offset, SEEK_SET) < 0 ||
        read(fd, shstrtab, shstrtab_hdr->sh_size) != (ssize_t)shstrtab_hdr->sh_size) {
        fprintf(stderr, "[ELF32 ERROR] Failed to read section header string table.\n");
        free(shstrtab);
        free(shdrs);
        close(fd);
        return -1;
    }

    // Locate .text section
    int text_idx = -1;
    for (int i = 0; i < ehdr.e_shnum; i++) {
        if (shdrs[i].sh_name < shstrtab_hdr->sh_size) {
            const char *name = shstrtab + shdrs[i].sh_name;
            if (strcmp(name, ".text") == 0) {
                text_idx = i;
                break;
            }
        }
    }

    if (text_idx == -1) {
        fprintf(stderr, "[ELF32 ERROR] .text section not found in ELF file.\n");
        free(shstrtab);
        free(shdrs);
        close(fd);
        return -1;
    }

    Elf32_Shdr *text_hdr = &shdrs[text_idx];
    sec->size = text_hdr->sh_size;
    sec->vaddr = text_hdr->sh_addr;

    sec->data = malloc(sec->size);
    if (!sec->data) {
        perror("[ELF32 ERROR] malloc text data");
        free(shstrtab);
        free(shdrs);
        close(fd);
        return -1;
    }

    if (lseek(fd, text_hdr->sh_offset, SEEK_SET) < 0 ||
        read(fd, sec->data, sec->size) != (ssize_t)sec->size) {
        fprintf(stderr, "[ELF32 ERROR] Failed to read .text section data.\n");
        free(sec->data);
        sec->data = NULL;
        free(shstrtab);
        free(shdrs);
        close(fd);
        return -1;
    }

    free(shstrtab);
    free(shdrs);
    close(fd);
    return 0;
}

void elf32_free_section(elf32_section_t *sec) {
    if (sec && sec->data) {
        free(sec->data);
        sec->data = NULL;
        sec->size = 0;
        sec->vaddr = 0;
    }
}
