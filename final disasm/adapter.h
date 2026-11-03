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

/* ========================================================================= */
/* 1. Control Flow & Instruction Classification                             */
/* ========================================================================= */

// How the instruction changes EIP (matches instruction.h.txt exactly)
typedef enum InstructionType {
    NORMAL = 0,              // Sequential execution: EIP += length
    CONDITIONAL_BRANCH,      // Conditional jump (e.g. JE, JNE, JG, JL)
    JUMP,                    // Unconditional jump (e.g. JMP)
    CALL,                    // Function call (pushes return address, jumps)
    RET                      // Function return (pops EIP from stack)
} InstructionType;

// Aliases for clear namespacing
#define INST_NORMAL          NORMAL
#define INST_COND_BRANCH     CONDITIONAL_BRANCH
#define INST_JUMP            JUMP
#define INST_CALL            CALL
#define INST_RET             RET

/* ========================================================================= */
/* 2. Condition Codes (for Jcc & SETcc Evaluation by Debugger)              */
/* ========================================================================= */

typedef enum ConditionCode {
    COND_NONE = 0,           // Non-conditional instruction
    COND_E,                  // Equal / Zero: ZF == 1
    COND_NE,                 // Not Equal / Not Zero: ZF == 0
    COND_L,                  // Less: SF != OF
    COND_LE,                 // Less or Equal: ZF == 1 || SF != OF
    COND_G,                  // Greater: ZF == 0 && SF == OF
    COND_GE,                 // Greater or Equal: SF == OF
    COND_B,                  // Below / Carry: CF == 1
    COND_BE,                 // Below or Equal: CF == 1 || ZF == 1
    COND_A,                  // Above: CF == 0 && ZF == 0
    COND_AE,                 // Above or Equal: CF == 0
    COND_S,                  // Sign: SF == 1
    COND_NS,                 // No Sign: SF == 0
    COND_O,                  // Overflow: OF == 1
    COND_NO,                 // No Overflow: OF == 0
    COND_P,                  // Parity: PF == 1
    COND_NP                  // No Parity: PF == 0
} ConditionCode;

/* ========================================================================= */
/* 3. Register Enums (x86 Hardware Encoding Order & Sub-Registers)           */
/* ========================================================================= */

// Registers in x86 encoding order, same numbers as the ModR/M fields
typedef enum RegName {
    REG_EAX = 0,
    REG_ECX = 1,
    REG_EDX = 2,
    REG_EBX = 3,
    REG_ESP = 4,
    REG_EBP = 5,
    REG_ESI = 6,
    REG_EDI = 7,
    REG_INVALID = 255        // No register
} RegName;

#define REG_COUNT 8
#define REG_NONE  REG_INVALID

// Backward compatibility alias for Register type
typedef RegName Register;

// Backward compatibility constants when reference framework is not included
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

/* ========================================================================= */
/* 4. Operand Width / Data Access Size                                       */
/* ========================================================================= */

typedef enum OperandWidth {
    WIDTH_UNKNOWN = 0,
    WIDTH_8_BIT   = 1,       // Byte (1 byte, e.g. %al, byte ptr)
    WIDTH_16_BIT  = 2,       // Word (2 bytes, e.g. %ax, word ptr)
    WIDTH_32_BIT  = 4        // Dword (4 bytes, e.g. %eax, dword ptr)
} OperandWidth;

/* ========================================================================= */
/* 5. Memory Operand: address = base + index*scale + disp                    */
/* ========================================================================= */

typedef struct MemOperand {
    RegName base;            // REG_INVALID if none
    RegName index;           // REG_INVALID if none
    uint8_t scale;           // 1, 2, 4 or 8 (0 if no index)
    union {
        int32_t disp;        // Displacement offset
        int32_t offset;      // Backward compatibility alias
    };
} MemOperand;

/* ========================================================================= */
/* 6. Unified Operand Representation                                         */
/* ========================================================================= */

typedef enum OperandType {
    OP_NONE = 0,
    OP_REGISTER,             // CPU Register operand
    OP_IMMEDIATE,            // Constant / Literal immediate operand
    OP_MEMORY                // Memory address [base + index*scale + disp]
} OperandType;

typedef enum OperandRole {
    ROLE_NONE = 0,
    ROLE_SRC,                // Read-only source operand
    ROLE_DST,                // Write-only destination operand
    ROLE_SRC_DST             // Read-write operand (e.g. destination in ADD/SUB)
} OperandRole;

typedef struct Operand {
    OperandType  type;       // OP_REGISTER, OP_IMMEDIATE, OP_MEMORY
    OperandWidth width;      // 1, 2, or 4 bytes
    OperandRole  role;       // Read / Write role

    union {
        RegName    reg;      // Valid when type == OP_REGISTER
        int32_t    immediate;// Valid when type == OP_IMMEDIATE
        MemOperand mem;      // Valid when type == OP_MEMORY
    };
} Operand;

/* ========================================================================= */
/* 7. Unified Instruction Structure                                          */
/* ========================================================================= */

typedef struct Instruction {
    // 7a. Addresses and Length (instruction.h.txt + compatibility)
    union {
        uint32_t start_address;  // Virtual address of the instruction
        uint32_t addr;           // Backward compatibility alias
    };
    union {
        size_t length;           // Size in bytes
        int    len;              // Backward compatibility alias
    };

    // 7b. Raw Decoded Information (instruction.h.txt + flat byte buffer)
    decoded_binary raw_decoded;  // Raw bytes & physical slices for listing
    uint8_t raw_bytes[MAX_RAW_BYTES]; // Direct flat byte array

    // 7c. Text & Mnemonic Representation
    char mnemonic[MAX_MNEMONIC_LEN]; // Lowercase name, e.g. "mov"
    char cmd[MAX_CMD_LEN];           // Uppercase name, e.g. "MOV" for backward compatibility
    char text[MAX_TEXT_LEN];         // Pre-formatted assembly: "mov    %esp,%ebp"
    char hex_str[MAX_HEX_LEN];       // Pre-formatted hex bytes: "89 e5"

    // 7d. Execution & Control Flow
    InstructionType type;        // NORMAL, CONDITIONAL_BRANCH, JUMP, CALL, RET
    ConditionCode   condition;   // Branch / SETcc condition code for EFLAGS
    uint32_t target_address;     // Absolute jump/call target
    bool     has_target_address; // True if target address is static & resolved
    bool     is_indirect;        // True if branch target is in register/memory (e.g. jmp *%eax)

    // 7e. Direct Convenience Fields (instruction.h.txt)
    RegName dest_reg;            // First operand register, REG_INVALID if none
    RegName src_reg;             // Second operand register, REG_INVALID if none
    int32_t immediate_val;       // Immediate value if present
    bool    has_immediate;       // True if instruction has immediate operand

    bool       is_memory_dest;   // First operand is memory
    MemOperand mem_dest;
    bool       is_memory_src;    // Second operand is memory
    MemOperand mem_src;

    // 7f. Array of Operands (for Debugger / Execution Engine)
    Operand      operands[MAX_OPERANDS];
    unsigned int n_operands;
} Instruction;

/* ========================================================================= */
/* 8. Disassembly Result Container                                           */
/* ========================================================================= */

typedef struct DisasmResult {
    Instruction *instructions;   // Dynamically allocated array of instructions
    unsigned int count;          // Total number of instructions decoded
    size_t       capacity;       // Internal buffer capacity
} DisasmResult;

/* ========================================================================= */
/* 9. Public API Functions                                                   */
/* ========================================================================= */

// Disassembles raw machine code into the unified Instruction format.
DisasmResult* disasm_to_adapter(const unsigned char *code, unsigned int size, unsigned int start_addr);

// Disassembles the .text section of a 32-bit ELF file directly.
DisasmResult* disasm_elf_file(const char *filepath);

// Frees the DisasmResult structure and all allocated instruction buffers.
void free_adapter_result(DisasmResult *result);

// Backward-compatibility aliases
#define disasm_to_passed_format    disasm_to_adapter
#define free_passed_format_result  free_adapter_result

#endif // ADAPTER_H
