#ifndef SEMANTIC_DECODER_H
#define SEMANTIC_DECODER_H

#include <stdint.h>
#include "decoded_binary_framework.h"
#include "reference_instruction_framework_V3.h"

// decode slices -> full instruction (returns 0 on success, -1 on error)
int decode_semantic(const decoded_binary *d, uint32_t addr, instruction_reference_t *inst);

#endif
