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

/***************************************************************************
 * [FEATURE ADDITION 2 - RAGINI]
 * FUTURE SCOPE FOR STEP DEBUGGER:
 * Function: decoder_has_memory_operand
 * Purpose : Inspects physical decoding fields (ModR/M, SIB, Displacement)
 *           to check if an instruction reads or writes memory.
 * Benefits: Allows debugger to intercept memory watchpoints and evaluate
 *           effective addresses before memory execution.
 * Returns : 1 if memory operand present, 0 if register/immediate only.
 ***************************************************************************/
int decoder_has_memory_operand(const decoded_binary *d);

#endif
