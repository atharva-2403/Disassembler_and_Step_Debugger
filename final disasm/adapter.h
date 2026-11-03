#ifndef ADAPTER_H
#define ADAPTER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "decoded_binary_framework.h"

#define MAX_RAW_BYTES    15
#define MAX_MNEMONIC_LEN 16
#define MAX_CMD_LEN      16
#define MAX_OPERANDS     3
#define MAX_TEXT_LEN     64
#define MAX_HEX_LEN      32

/* Instruction flow / branch category */
typedef enum InstructionType {
    NORMAL = 0,
    CONDITIONAL_BRANCH,
    JUMP,
    CALL,
    RET,
    INVALID
} InstructionType;

#define INST_NORMAL          NORMAL
#define INST_COND_BRANCH     CONDITIONAL_BRANCH
#define INST_JUMP            JUMP
#define INST_CALL            CALL
#define INST_RET             RET
#define INST_INVALID         INVALID

/* Condition codes for Jcc and SETcc instructions */
typedef enum ConditionCode {
    COND_NONE = 0,
    COND_E,
    COND_NE,
    COND_L,
    COND_LE,
    COND_G,
    COND_GE,
    COND_B,
    COND_BE,
    COND_A,
    COND_AE,
    COND_S,
    COND_NS,
    COND_O,
    COND_NO,
    COND_P,
    COND_NP
} ConditionCode;

/* x86 32-bit architectural registers */
typedef enum RegName {
    REG_EAX = 0,
    REG_ECX = 1,
    REG_EDX = 2,
    REG_EBX = 3,
    REG_ESP = 4,
    REG_EBP = 5,
    REG_ESI = 6,
    REG_EDI = 7,
    REG_INVALID = 255
} RegName;

#define REG_COUNT 8
#define REG_NONE  REG_INVALID

typedef RegName Register;

#ifndef REFERENCE_INSTRUCTION_FRAMEWORK_H
#define EAX REG_EAX
#define ECX REG_ECX
#define EDX REG_EDX
#define EBX REG_EBX
#define ESP REG_ESP
#define EBP REG_EBP
#define ESI REG_ESI
#define EDI REG_EDI
#endif

typedef enum OperandWidth {
    WIDTH_UNKNOWN = 0,
    WIDTH_8_BIT   = 1,
    WIDTH_16_BIT  = 2,
    WIDTH_32_BIT  = 4
} OperandWidth;

/* Memory address computation: [base + index*scale + disp] */
typedef struct MemOperand {
    RegName base;
    RegName index;
    uint8_t scale;
    union {
        int32_t disp;
        int32_t offset;
    };
} MemOperand;

typedef enum OperandType {
    OP_NONE = 0,
    OP_REGISTER,
    OP_IMMEDIATE,
    OP_MEMORY
} OperandType;

typedef enum OperandRole {
    ROLE_NONE = 0,
    ROLE_SRC,
    ROLE_DST,
    ROLE_SRC_DST
} OperandRole;

typedef struct Operand {
    OperandType  type;
    OperandWidth width;
    OperandRole  role;
    union {
        RegName    reg;
        int32_t    immediate;
        MemOperand mem;
    };
} Operand;

/* Main instruction structure for Frontend (Worklet 3) */
typedef struct Instruction {
    union {
        uint32_t start_address;
        uint32_t addr;
    };
    union {
        size_t length;
        int    len;
    };

    decoded_binary raw_decoded;
    uint8_t raw_bytes[MAX_RAW_BYTES];

    char mnemonic[MAX_MNEMONIC_LEN];
    char cmd[MAX_CMD_LEN];
    char text[MAX_TEXT_LEN];
    char hex_str[MAX_HEX_LEN];

    InstructionType type;
    ConditionCode   condition;
    bool            is_valid;
    uint32_t target_address;
    bool     has_target_address;
    bool     is_indirect;

    RegName dest_reg;
    RegName src_reg;
    int32_t immediate_val;
    bool    has_immediate;

    bool       is_memory_dest;
    MemOperand mem_dest;
    bool       is_memory_src;
    MemOperand mem_src;

    Operand      operands[MAX_OPERANDS];
    unsigned int n_operands;
} Instruction;

/* Frontend disassembler result container */
typedef struct DisasmResult {
    Instruction *instructions;
    unsigned int count;
    size_t       capacity;
} DisasmResult;

/* Worklet 2 (Execution / Debugger Engine) types */
typedef enum Worklet2OperandKind {
    W2_OP_NONE = 0,
    W2_OP_REG  = 1,
    W2_OP_IMM  = 2,
    W2_OP_MEM  = 3
} Worklet2OperandKind;

typedef struct worklet2_instruction {
    uint32_t address;
    uint8_t  length;
    char     mnemonic[16];
    uint8_t  flow;
    uint32_t target;
    uint8_t  operand_kind[2];
    uint8_t  operand_reg[2];
    int32_t  immediate;
} worklet2_instruction;

typedef struct decoded_program {
    uint32_t              load_base;
    uint32_t              entry_point;
    const uint8_t        *code;
    size_t                code_size;
    worklet2_instruction *insns;
    int                   insn_count;
} decoded_program;

/* --- Worklet 3 (Frontend API) --- */
DisasmResult* disasm_to_adapter(const unsigned char *code, unsigned int size, unsigned int start_addr);
DisasmResult* disasm_elf_file(const char *filepath);
void          free_adapter_result(DisasmResult *result);

/* --- Worklet 2 (Execution Engine API) --- */
decoded_program* disasm_for_worklet2(const char *filepath);
void             free_decoded_program(decoded_program *prog);

/* Aliases for older test scripts */
#define disasm_to_passed_format    disasm_to_adapter
#define free_passed_format_result  free_adapter_result

#endif /* ADAPTER_H */
