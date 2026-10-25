#ifndef REFERENCE_TABLE_STANDARD_H
#define REFERENCE_TABLE_STANDARD_H

#include <stdint.h>
#include "reference_instruction_standard.h"

typedef enum {
	 ENTRY_TYPE_NONE = 0,	
	 ENTRY_TYPE_SINGLE,       // Fixed: changed semicolon to comma
   	 ENTRY_TYPE_EXTENSION_GROUP   // Fixed: changed semicolon to comma
} entry_type_t;

typedef enum {
    TABLE_PRIMARY = 0,
    TABLE_SECONDARY,
    TABLE_TERTIARY_38,
    TABLE_TERTIARY_3A
} opcode_table_type_t;

typedef struct {
    entry_type_t type;
    union {
        instruction_reference_t single_instruction;
        instruction_reference_t extension_bucket[8];
    } content;
} opcode_table_entry_t;

void init_opcode_tables(void);

opcode_table_entry_t *get_opcode_entry(opcode_table_type_t table_type, uint8_t opcode);

#endif
