
#include "instruction_table.h"

const instruction_rule_t instruction_table[] = {
    // opcode, mask, mnemonic, immediate_size, fixed_length, type

    // Instructions with no operands
    {0x90, 0xFF, "nop", 0, 1, INSTR_OTHER},
    {0xC3, 0xFF, "ret", 0, 1, INSTR_RETURN},
    {0xC9, 0xFF, "leave", 0, 1, INSTR_OTHER},


    // PUSH register (50-57)
    {0x50, 0xF8, "push", 0, 1, INSTR_OTHER},

    // POP register (58-5F)
    {0x58, 0xF8, "pop", 0, 1, INSTR_OTHER},

    // MOV register, immediate (B8-BF)
    {0xB8, 0xF8, "mov", 4, 5, INSTR_OTHER},

    // ADD EAX, immediate
    {0x05, 0xFF, "add", 4, 5, INSTR_ARITHMETIC}
};

const unsigned int instruction_table_count =
    sizeof(instruction_table) / sizeof(instruction_table[0]);
