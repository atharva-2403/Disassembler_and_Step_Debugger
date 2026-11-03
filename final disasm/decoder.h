#ifndef DECODER_H
#define DECODER_H

#include "decoded_binary_framework.h"
#include <stdint.h>

void init_decoded(decoded_binary *d);
int decode_prefixes(uint8_t *bytes, int pos, int size, decoded_binary *d);
int decode_opcode(uint8_t *bytes, int pos, int size, decoded_binary *d);
int needs_modrm(decoded_binary *d);
int decode_modrm(uint8_t *bytes, int pos, int size, decoded_binary *d);
int needs_sib(decoded_binary *d);
int decode_sib(uint8_t *bytes, int pos, int size, decoded_binary *d);
int decode_disp(uint8_t *bytes, int pos, int size, decoded_binary *d);
int decode_immediate(uint8_t *bytes, int pos, int size, decoded_binary *d);
int decode_instruction(uint8_t *bytes, int pos, int size, decoded_binary *d);
void print_decoded(decoded_binary *d);

#endif
