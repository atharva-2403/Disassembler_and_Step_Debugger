#include "instruction_table.h"
#include <string.h>

// Helper macros to make table entries concise and readable
#define OP_RM(w, role)       { .encoding_type = ENCODING_RM,         .operand_width = (w), .operand_role = (role) }
#define OP_REG(w, role)      { .encoding_type = ENCODING_REG,        .operand_width = (w), .operand_role = (role) }
#define OP_OPREG(w, role)    { .encoding_type = ENCODING_OPCODE_REG, .operand_width = (w), .operand_role = (role) }
#define OP_IMM(w, role)      { .encoding_type = ENCODING_IMMEDIATE,  .operand_width = (w), .operand_role = (role) }
#define OP_REL(w, role)      { .encoding_type = ENCODING_RELATIVE,   .operand_width = (w), .operand_role = (role) }
#define OP_FIXED(r, w, role) { .encoding_type = ENCODING_FIXED_REG,  .operand_width = (w), .operand_value = { .reg = (r) }, .operand_role = (role) }

static const instruction_reference_t instruction_table[] = {

    // =========================================================================
    // 1. DATA MOVEMENT: MOV, LEA, PUSH, POP
    // =========================================================================

    // MOV r/m, reg
    { .opcode_bytes = {0x88}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "MOV", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_8, OPERAND_DESTINATION), OP_REG(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x89}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "MOV", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_DESTINATION), OP_REG(WIDTH_32, OPERAND_SOURCE) } },

    // MOV reg, r/m
    { .opcode_bytes = {0x8A}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "MOV", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_REG(WIDTH_8, OPERAND_DESTINATION), OP_RM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x8B}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "MOV", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_REG(WIDTH_32, OPERAND_DESTINATION), OP_RM(WIDTH_32, OPERAND_SOURCE) } },

    // MOV reg, imm (Opcode + reg: 0xB0+r, 0xB8+r)
    { .opcode_bytes = {0xB0}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "MOV", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_OPREG(WIDTH_8, OPERAND_DESTINATION), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0xB8}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "MOV", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_OPREG(WIDTH_32, OPERAND_DESTINATION), OP_IMM(WIDTH_32, OPERAND_SOURCE) } },

    // MOV r/m, imm (Group 11: 0xC6 /0, 0xC7 /0)
    { .opcode_bytes = {0xC6}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "MOV", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_0, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_8, OPERAND_DESTINATION), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0xC7}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "MOV", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_0, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_DESTINATION), OP_IMM(WIDTH_32, OPERAND_SOURCE) } },

    // LEA r32, m
    { .opcode_bytes = {0x8D}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "LEA", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_REG(WIDTH_32, OPERAND_DESTINATION), OP_RM(WIDTH_32, OPERAND_SOURCE) } },

    // PUSH
    { .opcode_bytes = {0x50}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "PUSH", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_OPREG(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x68}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "PUSH", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_IMM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x6A}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "PUSH", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_IMM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0xFF}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "PUSH", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_6, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_32, OPERAND_SOURCE) } },

    // POP
    { .opcode_bytes = {0x58}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "POP", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_OPREG(WIDTH_32, OPERAND_DESTINATION) } },
    { .opcode_bytes = {0x8F}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "POP", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_0, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_32, OPERAND_DESTINATION) } },

    // =========================================================================
    // 2. ARITHMETIC: ADD, SUB, INC, DEC, IMUL, NEG
    // =========================================================================

    // ADD
    { .opcode_bytes = {0x00}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "ADD", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_8, OPERAND_DESTINATION), OP_REG(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x01}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "ADD", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_DESTINATION), OP_REG(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x02}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "ADD", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_REG(WIDTH_8, OPERAND_DESTINATION), OP_RM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x03}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "ADD", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_REG(WIDTH_32, OPERAND_DESTINATION), OP_RM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x04}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "ADD", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_FIXED(AL, WIDTH_8, OPERAND_DESTINATION), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x05}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "ADD", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_FIXED(EAX, WIDTH_32, OPERAND_DESTINATION), OP_IMM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x80}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "ADD", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_0, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_8, OPERAND_DESTINATION), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x81}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "ADD", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_0, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_DESTINATION), OP_IMM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x83}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "ADD", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_0, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_DESTINATION), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },

    // SUB
    { .opcode_bytes = {0x28}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "SUB", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_8, OPERAND_DESTINATION), OP_REG(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x29}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "SUB", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_DESTINATION), OP_REG(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x2A}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "SUB", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_REG(WIDTH_8, OPERAND_DESTINATION), OP_RM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x2B}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "SUB", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_REG(WIDTH_32, OPERAND_DESTINATION), OP_RM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x2C}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "SUB", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_FIXED(AL, WIDTH_8, OPERAND_DESTINATION), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x2D}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "SUB", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_FIXED(EAX, WIDTH_32, OPERAND_DESTINATION), OP_IMM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x80}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "SUB", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_5, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_8, OPERAND_DESTINATION), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x81}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "SUB", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_5, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_DESTINATION), OP_IMM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x83}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "SUB", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_5, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_DESTINATION), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },

    // INC & DEC (Opcode + reg: 0x40+r, 0x48+r, and Group 4/5: FE/FF)
    { .opcode_bytes = {0x40}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "INC", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_OPREG(WIDTH_32, OPERAND_SRC_DEST) } },
    { .opcode_bytes = {0xFE}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "INC", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_0, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_8, OPERAND_SRC_DEST) } },
    { .opcode_bytes = {0xFF}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "INC", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_0, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_32, OPERAND_SRC_DEST) } },

    { .opcode_bytes = {0x48}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "DEC", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_OPREG(WIDTH_32, OPERAND_SRC_DEST) } },
    { .opcode_bytes = {0xFE}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "DEC", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_1, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_8, OPERAND_SRC_DEST) } },
    { .opcode_bytes = {0xFF}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "DEC", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_1, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_32, OPERAND_SRC_DEST) } },

    // IMUL
    { .opcode_bytes = {0xF7},       .opcode_size = ONE_B_OPCODE, .mnemonic_name = "IMUL", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_5, .operand_count = COUNT_ONE,   .operands = { OP_RM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x0F, 0xAF}, .opcode_size = TWO_B_OPCODE, .mnemonic_name = "IMUL", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO,   .operands = { OP_REG(WIDTH_32, OPERAND_DESTINATION), OP_RM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x69},       .opcode_size = ONE_B_OPCODE, .mnemonic_name = "IMUL", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_THREE, .operands = { OP_REG(WIDTH_32, OPERAND_DESTINATION), OP_RM(WIDTH_32, OPERAND_SOURCE), OP_IMM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x6B},       .opcode_size = ONE_B_OPCODE, .mnemonic_name = "IMUL", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_THREE, .operands = { OP_REG(WIDTH_32, OPERAND_DESTINATION), OP_RM(WIDTH_32, OPERAND_SOURCE), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },

    // NEG (Group 3: F6 /3, F7 /3)
    { .opcode_bytes = {0xF6}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "NEG", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_3, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_8, OPERAND_SRC_DEST) } },
    { .opcode_bytes = {0xF7}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "NEG", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_3, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_32, OPERAND_SRC_DEST) } },

    // =========================================================================
    // 3. DIVISION: IDIV, DIV, CDQ
    // =========================================================================

    // IDIV (Group 3: F6 /7, F7 /7)
    { .opcode_bytes = {0xF6}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "IDIV", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_7, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0xF7}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "IDIV", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_7, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_32, OPERAND_SOURCE) } },

    // DIV (Group 3: F6 /6, F7 /6)
    { .opcode_bytes = {0xF6}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "DIV", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_6, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0xF7}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "DIV", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_6, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_32, OPERAND_SOURCE) } },

    // CDQ (No operands)
    { .opcode_bytes = {0x99}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "CDQ", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ZERO },

    // =========================================================================
    // 4. LOGIC: AND, OR, XOR, NOT, TEST
    // =========================================================================

    // AND
    { .opcode_bytes = {0x20}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "AND", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_8, OPERAND_DESTINATION), OP_REG(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x21}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "AND", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_DESTINATION), OP_REG(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x22}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "AND", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_REG(WIDTH_8, OPERAND_DESTINATION), OP_RM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x23}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "AND", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_REG(WIDTH_32, OPERAND_DESTINATION), OP_RM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x24}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "AND", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_FIXED(AL, WIDTH_8, OPERAND_DESTINATION), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x25}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "AND", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_FIXED(EAX, WIDTH_32, OPERAND_DESTINATION), OP_IMM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x80}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "AND", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_4, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_8, OPERAND_DESTINATION), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x81}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "AND", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_4, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_DESTINATION), OP_IMM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x83}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "AND", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_4, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_DESTINATION), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },

    // OR
    { .opcode_bytes = {0x08}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "OR", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_8, OPERAND_DESTINATION), OP_REG(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x09}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "OR", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_DESTINATION), OP_REG(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x0A}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "OR", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_REG(WIDTH_8, OPERAND_DESTINATION), OP_RM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x0B}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "OR", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_REG(WIDTH_32, OPERAND_DESTINATION), OP_RM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x0C}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "OR", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_FIXED(AL, WIDTH_8, OPERAND_DESTINATION), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x0D}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "OR", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_FIXED(EAX, WIDTH_32, OPERAND_DESTINATION), OP_IMM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x80}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "OR", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_1, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_8, OPERAND_DESTINATION), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x81}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "OR", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_1, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_DESTINATION), OP_IMM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x83}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "OR", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_1, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_DESTINATION), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },

    // XOR
    { .opcode_bytes = {0x30}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "XOR", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_8, OPERAND_DESTINATION), OP_REG(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x31}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "XOR", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_DESTINATION), OP_REG(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x32}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "XOR", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_REG(WIDTH_8, OPERAND_DESTINATION), OP_RM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x33}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "XOR", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_REG(WIDTH_32, OPERAND_DESTINATION), OP_RM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x34}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "XOR", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_FIXED(AL, WIDTH_8, OPERAND_DESTINATION), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x35}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "XOR", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_FIXED(EAX, WIDTH_32, OPERAND_DESTINATION), OP_IMM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x80}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "XOR", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_6, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_8, OPERAND_DESTINATION), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x81}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "XOR", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_6, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_DESTINATION), OP_IMM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x83}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "XOR", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_6, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_DESTINATION), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },

    // NOT (Group 3: F6 /2, F7 /2)
    { .opcode_bytes = {0xF6}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "NOT", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_2, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_8, OPERAND_SRC_DEST) } },
    { .opcode_bytes = {0xF7}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "NOT", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_2, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_32, OPERAND_SRC_DEST) } },

    // TEST
    { .opcode_bytes = {0x84}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "TEST", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_8, OPERAND_SOURCE), OP_REG(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x85}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "TEST", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_SOURCE), OP_REG(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0xA8}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "TEST", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_FIXED(AL, WIDTH_8, OPERAND_SOURCE), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0xA9}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "TEST", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_FIXED(EAX, WIDTH_32, OPERAND_SOURCE), OP_IMM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0xF6}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "TEST", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_0, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_8, OPERAND_SOURCE), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0xF7}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "TEST", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_0, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_SOURCE), OP_IMM(WIDTH_32, OPERAND_SOURCE) } },

    // =========================================================================
    // 5. COMPARISON: CMP
    // =========================================================================

    { .opcode_bytes = {0x38}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "CMP", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_8, OPERAND_SOURCE), OP_REG(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x39}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "CMP", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_SOURCE), OP_REG(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x3A}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "CMP", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_REG(WIDTH_8, OPERAND_SOURCE), OP_RM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x3B}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "CMP", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_REG(WIDTH_32, OPERAND_SOURCE), OP_RM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x3C}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "CMP", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_FIXED(AL, WIDTH_8, OPERAND_SOURCE), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x3D}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "CMP", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_FIXED(EAX, WIDTH_32, OPERAND_SOURCE), OP_IMM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x80}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "CMP", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_7, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_8, OPERAND_SOURCE), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x81}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "CMP", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_7, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_SOURCE), OP_IMM(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x83}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "CMP", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_7, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_SOURCE), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },

    // =========================================================================
    // 6. CONTROL FLOW: JMP, Jcc, CALL, RET, LEAVE
    // =========================================================================

    // Unconditional JMP
    { .opcode_bytes = {0xEB}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "JMP", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_REL(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0xE9}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "JMP", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_REL(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0xFF}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "JMP", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_4, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_32, OPERAND_SOURCE) } },

    // Jcc Short (rel8: 0x70 - 0x7F)
    { .opcode_bytes = {0x74}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "JE",  .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_REL(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x75}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "JNE", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_REL(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x7F}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "JG",  .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_REL(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x7C}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "JL",  .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_REL(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x7D}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "JGE", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_REL(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x7E}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "JLE", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_REL(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x77}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "JA",  .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_REL(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x72}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "JB",  .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_REL(WIDTH_8, OPERAND_SOURCE) } },

    // Jcc Near (rel32: 0x0F 0x80 - 0x0F 0x8F)
    { .opcode_bytes = {0x0F, 0x84}, .opcode_size = TWO_B_OPCODE, .mnemonic_name = "JE",  .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_REL(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x0F, 0x85}, .opcode_size = TWO_B_OPCODE, .mnemonic_name = "JNE", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_REL(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x0F, 0x8F}, .opcode_size = TWO_B_OPCODE, .mnemonic_name = "JG",  .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_REL(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x0F, 0x8C}, .opcode_size = TWO_B_OPCODE, .mnemonic_name = "JL",  .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_REL(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x0F, 0x8D}, .opcode_size = TWO_B_OPCODE, .mnemonic_name = "JGE", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_REL(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x0F, 0x8E}, .opcode_size = TWO_B_OPCODE, .mnemonic_name = "JLE", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_REL(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x0F, 0x87}, .opcode_size = TWO_B_OPCODE, .mnemonic_name = "JA",  .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_REL(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0x0F, 0x82}, .opcode_size = TWO_B_OPCODE, .mnemonic_name = "JB",  .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_REL(WIDTH_32, OPERAND_SOURCE) } },

    // CALL
    { .opcode_bytes = {0xE8}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "CALL", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE, .operands = { OP_REL(WIDTH_32, OPERAND_SOURCE) } },
    { .opcode_bytes = {0xFF}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "CALL", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_2, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_32, OPERAND_SOURCE) } },

    // RET & LEAVE
    { .opcode_bytes = {0xC3}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "RET",   .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ZERO },
    { .opcode_bytes = {0xC2}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "RET",   .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ONE,  .operands = { OP_IMM(WIDTH_16, OPERAND_SOURCE) } },
    { .opcode_bytes = {0xC9}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "LEAVE", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ZERO },

    // =========================================================================
    // 7. SHIFT / ROTATE: SHL, SHR, SAR
    // =========================================================================

    // SHL/SAL (Group 2: reg = 4)
    { .opcode_bytes = {0xD1}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "SHL", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_4, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_SRC_DEST), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0xD3}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "SHL", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_4, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_SRC_DEST), OP_FIXED(CL, WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0xC1}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "SHL", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_4, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_SRC_DEST), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },

    // SHR (Group 2: reg = 5)
    { .opcode_bytes = {0xD1}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "SHR", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_5, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_SRC_DEST), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0xD3}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "SHR", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_5, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_SRC_DEST), OP_FIXED(CL, WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0xC1}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "SHR", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_5, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_SRC_DEST), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },

    // SAR (Group 2: reg = 7)
    { .opcode_bytes = {0xD1}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "SAR", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_7, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_SRC_DEST), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0xD3}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "SAR", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_7, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_SRC_DEST), OP_FIXED(CL, WIDTH_8, OPERAND_SOURCE) } },
    { .opcode_bytes = {0xC1}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "SAR", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_7, .operand_count = COUNT_TWO, .operands = { OP_RM(WIDTH_32, OPERAND_SRC_DEST), OP_IMM(WIDTH_8, OPERAND_SOURCE) } },

    // =========================================================================
    // 8. NO-OP / PADDING: NOP
    // =========================================================================

    { .opcode_bytes = {0x90},       .opcode_size = ONE_B_OPCODE, .mnemonic_name = "NOP", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ZERO },
    { .opcode_bytes = {0x0F, 0x1F}, .opcode_size = TWO_B_OPCODE, .mnemonic_name = "NOP", .modrm_needed = true,  .opcode_extension = OPCODE_EXTENSION_0, .operand_count = COUNT_ONE,  .operands = { OP_RM(WIDTH_32, OPERAND_SOURCE) } },

    // =========================================================================
    // 9. CONDITIONALLY INCLUDED: SETcc, MOVZX
    // =========================================================================

    // SETcc (0x0F 0x90 - 0x0F 0x9F)
    { .opcode_bytes = {0x0F, 0x94}, .opcode_size = TWO_B_OPCODE, .mnemonic_name = "SETE",  .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_0, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_8, OPERAND_DESTINATION) } },
    { .opcode_bytes = {0x0F, 0x95}, .opcode_size = TWO_B_OPCODE, .mnemonic_name = "SETNE", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_0, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_8, OPERAND_DESTINATION) } },
    { .opcode_bytes = {0x0F, 0x9F}, .opcode_size = TWO_B_OPCODE, .mnemonic_name = "SETG",  .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_0, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_8, OPERAND_DESTINATION) } },
    { .opcode_bytes = {0x0F, 0x9C}, .opcode_size = TWO_B_OPCODE, .mnemonic_name = "SETL",  .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_0, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_8, OPERAND_DESTINATION) } },
    { .opcode_bytes = {0x0F, 0x9D}, .opcode_size = TWO_B_OPCODE, .mnemonic_name = "SETGE", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_0, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_8, OPERAND_DESTINATION) } },
    { .opcode_bytes = {0x0F, 0x9E}, .opcode_size = TWO_B_OPCODE, .mnemonic_name = "SETLE", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_0, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_8, OPERAND_DESTINATION) } },
    { .opcode_bytes = {0x0F, 0x97}, .opcode_size = TWO_B_OPCODE, .mnemonic_name = "SETA",  .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_0, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_8, OPERAND_DESTINATION) } },
    { .opcode_bytes = {0x0F, 0x92}, .opcode_size = TWO_B_OPCODE, .mnemonic_name = "SETB",  .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_0, .operand_count = COUNT_ONE, .operands = { OP_RM(WIDTH_8, OPERAND_DESTINATION) } },

    // MOVZX
    { .opcode_bytes = {0x0F, 0xB6}, .opcode_size = TWO_B_OPCODE, .mnemonic_name = "MOVZX", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_REG(WIDTH_32, OPERAND_DESTINATION), OP_RM(WIDTH_8,  OPERAND_SOURCE) } },
    { .opcode_bytes = {0x0F, 0xB7}, .opcode_size = TWO_B_OPCODE, .mnemonic_name = "MOVZX", .modrm_needed = true, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_TWO, .operands = { OP_REG(WIDTH_32, OPERAND_DESTINATION), OP_RM(WIDTH_16, OPERAND_SOURCE) } },
};

static const size_t TABLE_COUNT = sizeof(instruction_table) / sizeof(instruction_table[0]);

size_t get_instruction_table_size(void) {
    return TABLE_COUNT;
}

const instruction_reference_t *get_instruction_table(void) {
    return instruction_table;
}

const instruction_reference_t *find_instruction_reference(const decoded_binary *d) {
    for (size_t i = 0; i < TABLE_COUNT; i++) {
        const instruction_reference_t *entry = &instruction_table[i];

        // 1. Check opcode size (1-byte vs 2-byte)
        if (entry->opcode_size != d->opcode.size) {
            continue;
        }

        // 2. Check if opcode embeds a register in its lower 3 bits (+rd form)
        bool has_opreg = false;
        for (int op = 0; op < (int)entry->operand_count; op++) {
            if (entry->operands[op].encoding_type == ENCODING_OPCODE_REG) {
                has_opreg = true;
                break;
            }
        }

        // 3. Match opcode byte(s)
        if (entry->opcode_size == ONE_B_OPCODE) {
            if (has_opreg) {
                // Low 3 bits hold register (0-7), high 5 bits must match base
                if ((d->opcode.bytes[0] & 0xF8) != entry->opcode_bytes[0]) {
                    continue;
                }
            } else {
                if (d->opcode.bytes[0] != entry->opcode_bytes[0]) {
                    continue;
                }
            }
        } else if (entry->opcode_size == TWO_B_OPCODE) {
            if (d->opcode.bytes[0] != entry->opcode_bytes[0] ||
                d->opcode.bytes[1] != entry->opcode_bytes[1]) {
                continue;
            }
        }

        // 4. Match ModR/M extension requirement (Group instructions like 80/81/83, FF, F6/F7)
        if (entry->modrm_needed) {
            if (d->modrm.is_valid != YES) {
                continue;
            }
            if (entry->opcode_extension != OPCODE_EXTENSION_R) {
                if ((int)d->modrm.reg != (int)entry->opcode_extension) {
                    continue;
                }
            }
        }

        // Found exact match!
        return entry;
    }

    return NULL; // Not found in table
}
