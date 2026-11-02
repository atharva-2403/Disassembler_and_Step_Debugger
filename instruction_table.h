
#ifndef INSTRUCTION_TABLE_H
#define INSTRUCTION_TABLE_H

#include <stdint.h>
#include <stdbool.h>
#include "instruction_framework.h"

// A single entry in the instruction lookup table.
// The decoder reads this to know both how to RECOGNISE an instruction (opcode + mask)
// and how to DECODE its operands (modrm_required, encoding, modrm_ext).
typedef struct {
    uint8_t  opcode;          // Base opcode byte to match against
    uint8_t  opcode_mask;     // Bitmask applied before comparing:
                              //   0xFF = exact match
                              //   0xF8 = ignore lower 3 bits (register is embedded in opcode)
    const char *mnemonic;     // Instruction name: "mov", "push", "nop" etc.

    uint8_t  immediate_size;  // Bytes of immediate data that follow (0 if none)
    uint8_t  fixed_length;    // Total bytes this instruction occupies: opcode + modrm + immediate

    instruction_type_t type;  // Category (INSTR_OTHER, INSTR_ARITHMETIC, INSTR_RETURN...)

    // His concept: these two fields make the decoder data-driven.
    // Instead of a strcmp() chain, the decoder switches on 'encoding' to print operands.
    bool     modrm_required;  // If true, a ModRM byte immediately follows the opcode
    operand_encoding_t encoding; // How to read and print this instruction's operands

    // His concept: extension groups — opcode 0x83 is shared by AND/SUB/CMP.
    // ModRM.reg field (bits [5:3]) disambiguates which one it is.
    //   -1  = no constraint (not an extension group)
    //  0-7  = ModRM.reg must equal this value for this rule to match
    int8_t   modrm_ext;
} instruction_rule_t;

// Defined in instruction_table.c
extern const instruction_rule_t instruction_table[];
extern const unsigned int instruction_table_count;

#endif
