
#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>
#include <stddef.h>
#include "instruction_framework.h"

int decode_instructions(const uint8_t *bytes, size_t size,
                        uint32_t start_address);

#endif
