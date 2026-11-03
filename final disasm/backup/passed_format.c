// To avoid C enum identifier collisions between passed_format.h and
// reference_instruction_framework_V3.h in the bridge file, we alias the
// passed_format enum symbols during inclusion. The teammates' files will
// include passed_format.h directly without this alias.
#define EAX PASSED_REG_EAX
#define EBX PASSED_REG_EBX
#define ECX PASSED_REG_ECX
#define EDX PASSED_REG_EDX
#define ESI PASSED_REG_ESI
#define EDI PASSED_REG_EDI
#define EBP PASSED_REG_EBP
#define ESP PASSED_REG_ESP
#include "passed_format.h"
#undef EAX
#undef EBX
#undef ECX
#undef EDX
#undef ESI
#undef EDI
#undef EBP
#undef ESP

#include "decoder.h"
#include "semantic_decoder.h"
#include "reference_instruction_framework_V3.h"
#include "decoded_binary_framework.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static Register map_to_passed_register(register_t reg) {
    switch (reg) {
        case AL: case AH: case AX: case EAX: return PASSED_REG_EAX;
        case BL: case BH: case BX: case EBX: return PASSED_REG_EBX;
        case CL: case CH: case CX: case ECX: return PASSED_REG_ECX;
        case DL: case DH: case DX: case EDX: return PASSED_REG_EDX;
        case SI: case ESI: return PASSED_REG_ESI;
        case DI: case EDI: return PASSED_REG_EDI;
        case BP: case EBP: return PASSED_REG_EBP;
        case SP: case ESP: return PASSED_REG_ESP;
        default: return PASSED_REG_EAX;
    }
}

static void map_operand(const decoded_binary *bin,
                        const operand_reference_t *src_op,
                        Operand *dst_op) {
    memset(dst_op, 0, sizeof(*dst_op));

    switch (src_op->encoding_type) {
        case ENCODING_REG:
        case ENCODING_OPCODE_REG:
        case ENCODING_FIXED_REG:
            dst_op->type = OP_REGISTER;
            dst_op->reg = map_to_passed_register(src_op->operand_value.reg);
            break;

        case ENCODING_RM:
            if (bin->modrm.mod == 3) {
                // Register operand encoded in r/m
                dst_op->type = OP_REGISTER;
                dst_op->reg = map_to_passed_register(src_op->operand_value.reg);
            } else {
                // Memory operand: base + displacement
                dst_op->type = OP_MEMORY;
                dst_op->mem.base = map_to_passed_register(src_op->operand_value.mem_val.base);
                dst_op->mem.offset = (int)src_op->operand_value.mem_val.displacement;
            }
            break;

        case ENCODING_IMMEDIATE:
            dst_op->type = OP_IMMEDIATE;
            dst_op->immediate = (int)src_op->operand_value.imm_val.value;
            break;

        case ENCODING_RELATIVE:
            // Branch/call relative targets mapped as immediate target addresses
            dst_op->type = OP_IMMEDIATE;
            dst_op->immediate = (int)src_op->operand_value.rel_val;
            break;

        default:
            break;
    }
}

DisasmResult* disasm_to_passed_format(const unsigned char *code, unsigned int size, unsigned int start_addr) {
    if (!code || size == 0) {
        return NULL;
    }

    DisasmResult *result = malloc(sizeof(DisasmResult));
    if (!result) {
        return NULL;
    }

    size_t capacity = 64;
    result->count = 0;
    result->instructions = malloc(sizeof(Instruction) * capacity);
    if (!result->instructions) {
        free(result);
        return NULL;
    }

    unsigned int offset = 0;

    while (offset < size) {
        if (result->count >= capacity) {
            size_t new_cap = capacity * 2;
            Instruction *new_insts = realloc(result->instructions, sizeof(Instruction) * new_cap);
            if (!new_insts) {
                // Out of memory: return whatever was decoded so far
                break;
            }
            result->instructions = new_insts;
            capacity = new_cap;
        }

        decoded_binary bin;
        unsigned int cur_addr = start_addr + offset;
        int slice_len = decode_instruction((uint8_t *)code, (int)offset, (int)size, &bin);

        if (slice_len > 0) {
            instruction_reference_t inst;
            if (decode_semantic(&bin, cur_addr, &inst) == 0) {
                Instruction *item = &result->instructions[result->count++];
                memset(item, 0, sizeof(*item));

                item->addr = cur_addr;
                item->len = bin.length;

                for (int i = 0; i < bin.length && i < MAX_RAW_BYTES; i++) {
                    item->raw_bytes[i] = (short)bin.raw_bytes[i];
                }

                if (inst.mnemonic_name) {
                    strncpy(item->cmd, inst.mnemonic_name, MAX_CMD_LEN - 1);
                    item->cmd[MAX_CMD_LEN - 1] = '\0';
                }

                item->n_operands = (unsigned int)inst.operand_count;
                for (unsigned int i = 0; i < item->n_operands && i < MAX_OPERANDS; i++) {
                    map_operand(&bin, &inst.operands[i], &item->operands[i]);
                }

                offset += bin.length;
                continue;
            }
        }

        // Fallback for bytes that failed decoding
        Instruction *item = &result->instructions[result->count++];
        memset(item, 0, sizeof(*item));
        item->addr = cur_addr;
        item->len = 1;
        item->raw_bytes[0] = (short)code[offset];
        strncpy(item->cmd, "BYTE", MAX_CMD_LEN - 1);
        item->n_operands = 1;
        item->operands[0].type = OP_IMMEDIATE;
        item->operands[0].immediate = (int)code[offset];

        offset += 1;
    }

    return result;
}

void free_passed_format_result(DisasmResult *result) {
    if (result) {
        if (result->instructions) {
            free(result->instructions);
            result->instructions = NULL;
        }
        free(result);
    }
}
