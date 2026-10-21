
#ifndef INSTRUCTION_TABLE_H
#define INSTRUCTION_TABLE_H

#include <stdint.h>
#include "instruction_framework.h"

//rules for recognizing supported instructions
typedef struct {
    uint8_t opcode;
    uint8_t opcode_mask;

    const char *mnemonic;

    uint8_t immediate_size;
    uint8_t fixed_length;

    instruction_type_t type;
} instruction_rule_t;

//the table will be defined in instruction_table.c
extern const instruction_rule_t instruction_table[];
extern const unsigned int instruction_table_count;

#endif
