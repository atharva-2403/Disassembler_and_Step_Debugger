#ifndef FORMATTER_H
#define FORMATTER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "decoded_binary_framework.h"
#include "reference_instruction_framework_V3.h"

// Formats a decoded instruction into standard AT&T assembly syntax matching objdump -d.
// Produces a full line containing: address, hex bytes, mnemonic, and operands.
// Example: " 1000:\t53                  \tpush   %ebx"
int format_instruction_att(const decoded_binary *bin,
                           const instruction_reference_t *inst,
                           uint32_t addr,
                           char *buffer,
                           size_t max_len);

// Formats only the assembly instruction (mnemonic + operands), e.g. "mov    %esp,%ebp"
int format_assembly_only(const decoded_binary *bin,
                         const instruction_reference_t *inst,
                         char *buffer,
                         size_t max_len);

#endif // FORMATTER_H
