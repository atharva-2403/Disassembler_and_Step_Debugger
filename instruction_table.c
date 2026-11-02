
#include "instruction_table.h"

// The instruction lookup table.
// Each row describes one instruction form. The decoder scans this array,
// matches opcode & mask, then uses modrm_required/encoding/modrm_ext to decode.
//
// Columns: opcode | mask | mnemonic | imm_bytes | total_len | type | modrm? | encoding | ext
const instruction_rule_t instruction_table[] = {

    // --- No-operand instructions (your originals) ---
    // Mask 0xFF = exact byte match. ENC_NONE = just print the mnemonic.
    {0x90, 0xFF, "nop",   0, 1, INSTR_OTHER,     false, ENC_NONE,         -1},
    {0xC3, 0xFF, "ret",   0, 1, INSTR_RETURN,     false, ENC_NONE,         -1},
    {0xC9, 0xFF, "leave", 0, 1, INSTR_OTHER,      false, ENC_NONE,         -1},

    // --- Register encoded in opcode lower 3 bits (your originals) ---
    // Mask 0xF8 ignores bits [2:0]. ENC_OPCODE_REG: get_register(opcode) extracts the register.
    // PUSH: opcodes 0x50(EAX) 0x51(ECX) ... 0x57(EDI)
    {0x50, 0xF8, "push",  0, 1, INSTR_OTHER,      false, ENC_OPCODE_REG,   -1},
    // POP: opcodes 0x58(EAX) 0x59(ECX) ... 0x5F(EDI)
    {0x58, 0xF8, "pop",   0, 1, INSTR_OTHER,      false, ENC_OPCODE_REG,   -1},

    // --- Register in opcode + 32-bit immediate (your original) ---
    // ENC_OPCODE_IMM32: register from opcode[2:0], then 4 bytes of immediate.
    // MOV B8+rd, imm32: opcodes 0xB8(EAX) ... 0xBF(EDI), length = 1 opcode + 4 imm = 5
    {0xB8, 0xF8, "mov",   4, 5, INSTR_OTHER,      false, ENC_OPCODE_IMM32, -1},

    // --- Fixed register + 32-bit immediate (your original) ---
    // ENC_IMM32: destination is always EAX, immediate follows.
    // ADD EAX, imm32 (opcode 0x05), length = 1 + 4 = 5
    {0x05, 0xFF, "add",   4, 5, INSTR_ARITHMETIC, false, ENC_IMM32,        -1},

    // --- ModRM register-to-register moves (his concept) ---
    // These need a ModRM byte to identify both registers.
    // MOV r/m32, r32 (89 /r): rm=destination, reg=source. length = 1 opcode + 1 modrm = 2
    {0x89, 0xFF, "mov",   0, 2, INSTR_OTHER,      true,  ENC_RM_REG,       -1},
    // MOV r32, r/m32 (8B /r): reg=destination, rm=source. length = 1 + 1 = 2
    {0x8B, 0xFF, "mov",   0, 2, INSTR_OTHER,      true,  ENC_REG_RM,       -1},

    // --- Extension group 0x83 (his concept) ---
    // Opcode 0x83 is shared. ModRM.reg field (bits [5:3]) picks the actual instruction:
    //   /4 = AND   /5 = SUB   /7 = CMP
    // Each takes rm register + sign-extended 8-bit immediate. length = 1 + 1 modrm + 1 imm8 = 3
    {0x83, 0xFF, "and",   1, 3, INSTR_ARITHMETIC, true,  ENC_RM_IMM8,       4},
    {0x83, 0xFF, "sub",   1, 3, INSTR_ARITHMETIC, true,  ENC_RM_IMM8,       5},
    {0x83, 0xFF, "cmp",   1, 3, INSTR_ARITHMETIC, true,  ENC_RM_IMM8,       7},
};

const unsigned int instruction_table_count =
    sizeof(instruction_table) / sizeof(instruction_table[0]);
