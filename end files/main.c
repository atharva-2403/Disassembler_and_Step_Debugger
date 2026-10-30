#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "elf32.h"
#include "decoder.h"
#include "semantic_decoder.h"
#include "formatter.h"

static void disassemble_stream(const uint8_t *code, size_t size, uint32_t base_vaddr) {
    size_t offset = 0;

    while (offset < size) {
        decoded_binary bin;
        uint32_t cur_addr = base_vaddr + (uint32_t)offset;
        int slice_len = decode_instruction((uint8_t *)code, (int)offset, (int)size, &bin);

        if (slice_len > 0) {
            instruction_reference_t inst;
            if (decode_semantic(&bin, cur_addr, &inst) == 0) {
                char line[256];
                format_instruction_att(&bin, &inst, cur_addr, line, sizeof(line));
                printf("%s\n", line);
                offset += bin.length;
                continue;
            }
        }

        // Fallback if slicing or semantic decode fails: emit single .byte like GNU objdump
        printf("    %4x:\t%-20.2x\t.byte 0x%02x\n",
               cur_addr, code[offset], code[offset]);
        offset += 1;
    }
}

static void print_usage(const char *prog_name) {
    printf("Usage:\n");
    printf("  %s <elf32_binary>               Disassemble .text section of a 32-bit ELF\n", prog_name);
    printf("  %s -b <hex_byte1> [byte2 ...]   Disassemble raw machine code bytes\n", prog_name);
    printf("\nExamples:\n");
    printf("  %s elf32\n", prog_name);
    printf("  %s -b 55 89 e5 83 ec 10 8d 45 08 c3\n", prog_name);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
        return 0;
    }

    // Mode 1: Disassemble raw bytes from command line arguments
    if (strcmp(argv[1], "-b") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: -b requires at least one byte in hex (e.g. 55 89 e5)\n");
            return 1;
        }

        size_t num_bytes = (size_t)(argc - 2);
        uint8_t *raw_buf = malloc(num_bytes);
        if (!raw_buf) {
            perror("malloc");
            return 1;
        }

        for (int i = 2; i < argc; i++) {
            unsigned int byte_val = 0;
            if (sscanf(argv[i], "%x", &byte_val) != 1 || byte_val > 0xFF) {
                fprintf(stderr, "Error: Invalid hex byte '%s'\n", argv[i]);
                free(raw_buf);
                return 1;
            }
            raw_buf[i - 2] = (uint8_t)byte_val;
        }

        printf("\nDisassembly of raw bytes:\n\n");
        disassemble_stream(raw_buf, num_bytes, 0x08048000);
        printf("\n");

        free(raw_buf);
        return 0;
    }

    // Mode 2: Disassemble 32-bit ELF file
    const char *elf_path = argv[1];
    elf32_section_t text_sec;

    if (elf32_load_text_section(elf_path, &text_sec) != 0) {
        fprintf(stderr, "Failed to load .text section from '%s'\n", elf_path);
        return 1;
    }

    printf("\n%s:     file format elf32-i386\n\n", elf_path);
    printf("Disassembly of section .text:\n\n");

    disassemble_stream(text_sec.data, text_sec.size, text_sec.vaddr);
    printf("\n");

    elf32_free_section(&text_sec);
    return 0;
}
