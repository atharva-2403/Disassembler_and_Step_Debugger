#ifndef PASSED_FORMAT_H
#define PASSED_FORMAT_H

#define MAX_RAW_BYTES 15
#define MAX_CMD_LEN   16
#define MAX_OPERANDS   3

typedef enum Register {
    EAX,
    EBX,
    ECX,
    EDX,
    ESI,
    EDI,
    EBP,
    ESP
} Register;

typedef enum OperandType {
    OP_REGISTER,
    OP_IMMEDIATE,
    OP_MEMORY
} OperandType;

typedef struct Operand {
    OperandType type; // addressing mode

    union {
        Register reg;      // if type == OP_REGISTER
        int immediate;     // if type == OP_IMMEDIATE
        struct mem {
            Register base;
            int offset;     // if type == OP_MEMORY
        } mem;
    };
} Operand;

typedef struct Instruction {
    unsigned int addr;              // address in memory
    short raw_bytes[MAX_RAW_BYTES]; // e.g. {1, 0, 0, 1, ..}
    int len;                        // length of instruction
    char cmd[MAX_CMD_LEN];          // e.g. "MOV", "JMP", etc.
    Operand operands[MAX_OPERANDS];
    unsigned int n_operands;        // no. of operands
} Instruction;

typedef struct DisasmResult {
    Instruction *instructions;
    unsigned int count; // no. of instr.
} DisasmResult;

/* API Functions for other modules/teams to consume */
DisasmResult* disasm_to_passed_format(const unsigned char *code, unsigned int size, unsigned int start_addr);
void free_passed_format_result(DisasmResult *result);

#endif // PASSED_FORMAT_H
