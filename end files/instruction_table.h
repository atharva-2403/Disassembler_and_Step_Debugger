#ifndef INSTRUCTION_TABLE_H
#define INSTRUCTION_TABLE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "decoded_binary_framework.h"
#include "reference_instruction_framework_V3.h"

// Looks up an instruction template from the table based on the sliced binary.
// Matches opcode bytes, embedded opcode registers (+rd), and ModR/M reg extensions.
// Returns a pointer to the matching template in the table, or NULL if unsupported.
const instruction_reference_t *find_instruction_reference(const decoded_binary *d);

// Returns total number of instruction entries in the table
size_t get_instruction_table_size(void);

// Returns direct pointer to the table array
const instruction_reference_t *get_instruction_table(void);

#endif // INSTRUCTION_TABLE_H
