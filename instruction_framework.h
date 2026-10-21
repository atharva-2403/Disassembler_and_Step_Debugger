
#ifndef INSTRUCTION_FRAMEWORK_H
#define INSTRUCTION_FRAMEWORK_H

#include <stdint.h>

//registers available in 32 bit x86
typedef enum {
    REG_NONE,
    REG_EAX, REG_ECX, REG_EDX, REG_EBX,
    REG_ESP, REG_EBP, REG_ESI, REG_EDI
} x86_register_t;

//types of values an instruction can use
typedef enum {
    OPERAND_NONE,
    OPERAND_REGISTER,
    OPERAND_IMMEDIATE
} operand_type_t;

//one operand of an instruction
typedef struct {
    operand_type_t type;
    x86_register_t reg;
    uint32_t immediate;
} operand_t;

//the instruction's category
typedef enum {
    INSTR_OTHER,
    INSTR_ARITHMETIC,
    INSTR_JUMP,
    INSTR_CALL,
    INSTR_RETURN
} instruction_type_t;

//information about one decoded instruction
typedef struct {
    uint32_t address;
    uint8_t bytes[15];
    uint8_t length;

    const char *mnemonic;
operand_t operands[2];
uint8_t operand_count;

 instruction_type_t type;
} decoded_instruction_t;

#endif
