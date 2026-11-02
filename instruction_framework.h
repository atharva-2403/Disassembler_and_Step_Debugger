
#ifndef INSTRUCTION_FRAMEWORK_H
#define INSTRUCTION_FRAMEWORK_H

#include <stdint.h>
#include <stdbool.h>

//registers available in 32 bit x86
typedef enum {
    REG_NONE,
    REG_EAX, REG_ECX, REG_EDX, REG_EBX,
    REG_ESP, REG_EBP, REG_ESI, REG_EDI,
    // 16-bit variants — used when the OPSIZE prefix (0x66) is present
    // His concept: resolveModrmReg() picks from this set when opsize_override is true
    REG_AX, REG_CX, REG_DX, REG_BX,
    REG_SP, REG_BP, REG_SI, REG_DI
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

// Describes how an instruction's operands are encoded in the byte stream.
// His concept: instead of checking mnemonic with strcmp(), the decoder switches
// on this value to know exactly how to read and print the operands.
//
//   ENC_NONE         — no operands:                      nop, ret, leave
//   ENC_OPCODE_REG   — register in opcode bits [2:0]:    push rX, pop rX
//   ENC_OPCODE_IMM32 — opcode[2:0] register + imm32:    mov rX, imm32  (B8+rd)
//   ENC_IMM32        — fixed eax + 32-bit immediate:     add eax, imm32
//   ENC_RM_REG       — ModRM: rm=dst, reg=src:           mov r/m32, r32 (89 /r)
//   ENC_REG_RM       — ModRM: reg=dst, rm=src:           mov r32, r/m32 (8B /r)
//   ENC_RM_IMM8      — ModRM rm + sign-extended imm8:    and/sub/cmp r/m32, imm8 (83 /x)
typedef enum {
    ENC_NONE,
    ENC_OPCODE_REG,
    ENC_OPCODE_IMM32,
    ENC_IMM32,
    ENC_RM_REG,
    ENC_REG_RM,
    ENC_RM_IMM8
} operand_encoding_t;

#endif
