#include "reference_instruction_framework_V3.h"
#include "adapter.h"
#include "decoder.h"
#include "semantic_decoder.h"
#include "decoded_binary_framework.h"
#include "formatter.h"
#include "elf32.h"

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdio.h>

/* ========================================================================= */
/* Helper: Map Internal Decoder Register Enum to Hardware-Aligned RegName   */
/* ========================================================================= */

static RegName map_register(register_t reg) {
    switch (reg) {
        // 8-bit, 16-bit, and 32-bit registers mapped to their base architectural register ID
        case AL: case AH: case AX: case EAX: return REG_EAX;
        case CL: case CH: case CX: case ECX: return REG_ECX;
        case DL: case DH: case DX: case EDX: return REG_EDX;
        case BL: case BH: case BX: case EBX: return REG_EBX;
        case SP: case ESP:                   return REG_ESP;
        case BP: case EBP:                   return REG_EBP;
        case SI: case ESI:                   return REG_ESI;
        case DI: case EDI:                   return REG_EDI;
        default:                             return REG_INVALID;
    }
}

/* ========================================================================= */
/* Helper: Map Operand Width to Standard Byte Size                           */
/* ========================================================================= */

static OperandWidth map_width(operand_width_t width) {
    switch (width) {
        case WIDTH_8:  return WIDTH_8_BIT;
        case WIDTH_16: return WIDTH_16_BIT;
        case WIDTH_32: return WIDTH_32_BIT;
        default:       return WIDTH_UNKNOWN;
    }
}

/* ========================================================================= */
/* Helper: Map Operand Role (Read / Write Semantics)                         */
/* ========================================================================= */

static OperandRole map_role(operand_role_t role) {
    switch (role) {
        case OPERAND_SOURCE:      return ROLE_SRC;
        case OPERAND_DESTINATION: return ROLE_DST;
        case OPERAND_SRC_DEST:    return ROLE_SRC_DST;
        default:                  return ROLE_NONE;
    }
}

/* ========================================================================= */
/* Helper: Classify Instruction Flow, Conditions, and Branch Types          */
/* ========================================================================= */

static void classify_instruction(const char *mnemonic,
                                 const operand_reference_t *op0,
                                 InstructionType *out_type,
                                 ConditionCode *out_cond,
                                 bool *out_is_indirect) {
    *out_type = NORMAL;
    *out_cond = COND_NONE;
    *out_is_indirect = false;

    if (!mnemonic) {
        return;
    }

    // Return instruction
    if (strcasecmp(mnemonic, "ret") == 0) {
        *out_type = RET;
        return;
    }

    // Call instruction
    if (strcasecmp(mnemonic, "call") == 0) {
        *out_type = CALL;
        if (op0 && (op0->encoding_type == ENCODING_REG || op0->encoding_type == ENCODING_RM)) {
            *out_is_indirect = true;
        }
        return;
    }

    // Unconditional Jump
    if (strcasecmp(mnemonic, "jmp") == 0) {
        *out_type = JUMP;
        if (op0 && (op0->encoding_type == ENCODING_REG || op0->encoding_type == ENCODING_RM)) {
            *out_is_indirect = true;
        }
        return;
    }

    // Conditional Branches (Jcc)
    if ((mnemonic[0] == 'j' || mnemonic[0] == 'J') && strlen(mnemonic) >= 2) {
        *out_type = CONDITIONAL_BRANCH;
        const char *cc = mnemonic + 1;

        if (strcasecmp(cc, "e") == 0 || strcasecmp(cc, "z") == 0) {
            *out_cond = COND_E;
        } else if (strcasecmp(cc, "ne") == 0 || strcasecmp(cc, "nz") == 0) {
            *out_cond = COND_NE;
        } else if (strcasecmp(cc, "l") == 0 || strcasecmp(cc, "nge") == 0) {
            *out_cond = COND_L;
        } else if (strcasecmp(cc, "le") == 0 || strcasecmp(cc, "ng") == 0) {
            *out_cond = COND_LE;
        } else if (strcasecmp(cc, "g") == 0 || strcasecmp(cc, "nle") == 0) {
            *out_cond = COND_G;
        } else if (strcasecmp(cc, "ge") == 0 || strcasecmp(cc, "nl") == 0) {
            *out_cond = COND_GE;
        } else if (strcasecmp(cc, "b") == 0 || strcasecmp(cc, "nae") == 0 || strcasecmp(cc, "c") == 0) {
            *out_cond = COND_B;
        } else if (strcasecmp(cc, "be") == 0 || strcasecmp(cc, "na") == 0) {
            *out_cond = COND_BE;
        } else if (strcasecmp(cc, "a") == 0 || strcasecmp(cc, "nbe") == 0) {
            *out_cond = COND_A;
        } else if (strcasecmp(cc, "ae") == 0 || strcasecmp(cc, "nb") == 0 || strcasecmp(cc, "nc") == 0) {
            *out_cond = COND_AE;
        } else if (strcasecmp(cc, "s") == 0) {
            *out_cond = COND_S;
        } else if (strcasecmp(cc, "ns") == 0) {
            *out_cond = COND_NS;
        } else if (strcasecmp(cc, "o") == 0) {
            *out_cond = COND_O;
        } else if (strcasecmp(cc, "no") == 0) {
            *out_cond = COND_NO;
        } else if (strcasecmp(cc, "p") == 0 || strcasecmp(cc, "pe") == 0) {
            *out_cond = COND_P;
        } else if (strcasecmp(cc, "np") == 0 || strcasecmp(cc, "po") == 0) {
            *out_cond = COND_NP;
        }
        return;
    }

    // Conditional Set (SETcc)
    if (strncasecmp(mnemonic, "set", 3) == 0 && strlen(mnemonic) >= 4) {
        const char *cc = mnemonic + 3;
        if (strcasecmp(cc, "e") == 0 || strcasecmp(cc, "z") == 0)        *out_cond = COND_E;
        else if (strcasecmp(cc, "ne") == 0 || strcasecmp(cc, "nz") == 0) *out_cond = COND_NE;
        else if (strcasecmp(cc, "l") == 0 || strcasecmp(cc, "nge") == 0) *out_cond = COND_L;
        else if (strcasecmp(cc, "le") == 0 || strcasecmp(cc, "ng") == 0) *out_cond = COND_LE;
        else if (strcasecmp(cc, "g") == 0 || strcasecmp(cc, "nle") == 0) *out_cond = COND_G;
        else if (strcasecmp(cc, "ge") == 0 || strcasecmp(cc, "nl") == 0) *out_cond = COND_GE;
        else if (strcasecmp(cc, "b") == 0)                               *out_cond = COND_B;
        else if (strcasecmp(cc, "be") == 0)                              *out_cond = COND_BE;
        else if (strcasecmp(cc, "a") == 0)                               *out_cond = COND_A;
        else if (strcasecmp(cc, "ae") == 0)                              *out_cond = COND_AE;
    }
}

/* ========================================================================= */
/* Helper: Translate Internal Operands to Unified Format                    */
/* ========================================================================= */

static void map_operand(const decoded_binary *bin,
                        const operand_reference_t *src_op,
                        Operand *dst_op) {
    memset(dst_op, 0, sizeof(*dst_op));

    dst_op->width = map_width(src_op->operand_width);
    dst_op->role  = map_role(src_op->operand_role);

    switch (src_op->encoding_type) {
        case ENCODING_REG:
        case ENCODING_OPCODE_REG:
        case ENCODING_FIXED_REG:
            dst_op->type = OP_REGISTER;
            dst_op->reg  = map_register(src_op->operand_value.reg);
            break;

        case ENCODING_RM:
            if (bin->modrm.mod == 3) {
                // Register operand encoded in ModR/M r/m field
                dst_op->type = OP_REGISTER;
                dst_op->reg  = map_register(src_op->operand_value.reg);
            } else {
                // Full effective memory address: base + index * scale + disp
                dst_op->type = OP_MEMORY;
                dst_op->mem.base  = map_register(src_op->operand_value.mem_val.base);
                dst_op->mem.index = map_register(src_op->operand_value.mem_val.index);
                dst_op->mem.scale = src_op->operand_value.mem_val.scale ? src_op->operand_value.mem_val.scale : 1;
                dst_op->mem.disp  = src_op->operand_value.mem_val.displacement;
            }
            break;

        case ENCODING_IMMEDIATE:
            dst_op->type = OP_IMMEDIATE;
            dst_op->immediate = (int32_t)src_op->operand_value.imm_val.value;
            break;

        case ENCODING_RELATIVE:
            // Relative branch/call target resolved into an absolute memory address
            dst_op->type = OP_IMMEDIATE;
            dst_op->immediate = (int32_t)src_op->operand_value.rel_val;
            break;

        default:
            dst_op->type = OP_NONE;
            break;
    }
}

/* ========================================================================= */
/* Main API: Disassemble Byte Stream into Unified Adapter Format            */
/* ========================================================================= */

DisasmResult* disasm_to_adapter(const unsigned char *code, unsigned int size, unsigned int start_addr) {
    if (!code || size == 0) {
        return NULL;
    }

    DisasmResult *result = malloc(sizeof(DisasmResult));
    if (!result) {
        return NULL;
    }

    result->capacity = 64;
    result->count = 0;
    result->instructions = malloc(sizeof(Instruction) * result->capacity);
    if (!result->instructions) {
        free(result);
        return NULL;
    }

    unsigned int offset = 0;

    while (offset < size) {
        // Expand storage buffer dynamically if needed
        if (result->count >= result->capacity) {
            size_t new_cap = result->capacity * 2;
            Instruction *new_insts = realloc(result->instructions, sizeof(Instruction) * new_cap);
            if (!new_insts) {
                // Out of memory: preserve whatever was successfully disassembled so far
                break;
            }
            result->instructions = new_insts;
            result->capacity = new_cap;
        }

        decoded_binary bin;
        unsigned int cur_addr = start_addr + offset;
        int slice_len = decode_instruction((uint8_t *)code, (int)offset, (int)size, &bin);

        if (slice_len > 0) {
            instruction_reference_t inst;
            if (decode_semantic(&bin, cur_addr, &inst) == 0) {
                Instruction *item = &result->instructions[result->count++];
                memset(item, 0, sizeof(*item));

                // 1. Basic Metadata (instruction.h.txt + compatibility)
                item->start_address = cur_addr;
                item->length        = bin.length;

                // 2. Raw Decoded Information & Hex String
                item->raw_decoded = bin; // Full physical slicer struct for listing
                size_t hex_pos = 0;
                for (int i = 0; i < bin.length && i < MAX_RAW_BYTES; i++) {
                    item->raw_bytes[i] = bin.raw_bytes[i];
                    if (hex_pos < sizeof(item->hex_str) - 4) {
                        hex_pos += (size_t)snprintf(item->hex_str + hex_pos,
                                                   sizeof(item->hex_str) - hex_pos,
                                                   "%02x ", bin.raw_bytes[i]);
                    }
                }
                if (hex_pos > 0 && item->hex_str[hex_pos - 1] == ' ') {
                    item->hex_str[hex_pos - 1] = '\0';
                }

                // 3. Mnemonics (lowercase for frontend, uppercase for cmd compatibility)
                if (inst.mnemonic_name) {
                    size_t mlen = strlen(inst.mnemonic_name);
                    for (size_t i = 0; i < mlen && i < MAX_MNEMONIC_LEN - 1; i++) {
                        item->mnemonic[i] = (char)tolower((unsigned char)inst.mnemonic_name[i]);
                    }
                    item->mnemonic[mlen < MAX_MNEMONIC_LEN ? mlen : MAX_MNEMONIC_LEN - 1] = '\0';

                    for (size_t i = 0; i < mlen && i < MAX_CMD_LEN - 1; i++) {
                        item->cmd[i] = (char)toupper((unsigned char)inst.mnemonic_name[i]);
                    }
                    item->cmd[mlen < MAX_CMD_LEN ? mlen : MAX_CMD_LEN - 1] = '\0';
                }

                // 4. Pre-formatted Assembly String for Frontend
                format_assembly_only(&bin, &inst, item->text, sizeof(item->text));

                // 5. Initialize Convenience Fields (instruction.h.txt)
                item->dest_reg        = REG_INVALID;
                item->src_reg         = REG_INVALID;
                item->mem_dest.base   = REG_INVALID;
                item->mem_dest.index  = REG_INVALID;
                item->mem_src.base    = REG_INVALID;
                item->mem_src.index   = REG_INVALID;

                // 6. Map Operands Array & Populate Convenience Fields
                item->n_operands = (unsigned int)inst.operand_count;

                for (unsigned int i = 0; i < item->n_operands && i < MAX_OPERANDS; i++) {
                    map_operand(&bin, &inst.operands[i], &item->operands[i]);

                    // Check for relative branch target address
                    if (inst.operands[i].encoding_type == ENCODING_RELATIVE) {
                        item->target_address = (uint32_t)inst.operands[i].operand_value.rel_val;
                        item->has_target_address = true;
                    }

                    // Check for immediate operand
                    if (item->operands[i].type == OP_IMMEDIATE) {
                        item->immediate_val = item->operands[i].immediate;
                        item->has_immediate = true;
                    }
                }

                // Populate operand 0 fields (dest_reg / mem_dest)
                if (item->n_operands >= 1) {
                    if (item->operands[0].type == OP_REGISTER) {
                        item->dest_reg = item->operands[0].reg;
                    } else if (item->operands[0].type == OP_MEMORY) {
                        item->is_memory_dest = true;
                        item->mem_dest = item->operands[0].mem;
                    }
                }

                // Populate operand 1 fields (src_reg / mem_src)
                if (item->n_operands >= 2) {
                    if (item->operands[1].type == OP_REGISTER) {
                        item->src_reg = item->operands[1].reg;
                    } else if (item->operands[1].type == OP_MEMORY) {
                        item->is_memory_src = true;
                        item->mem_src = item->operands[1].mem;
                    }
                }

                // 7. Control Flow & Condition Evaluation
                classify_instruction(inst.mnemonic_name,
                                     item->n_operands > 0 ? &inst.operands[0] : NULL,
                                     &item->type,
                                     &item->condition,
                                     &item->is_indirect);

                offset += bin.length;
                continue;
            }
        }

        // Fallback for undecodable machine bytes
        Instruction *item = &result->instructions[result->count++];
        memset(item, 0, sizeof(*item));

        item->start_address = cur_addr;
        item->length        = 1;
        item->raw_bytes[0]  = code[offset];
        item->raw_decoded.length = 1;
        item->raw_decoded.raw_bytes[0] = code[offset];

        snprintf(item->hex_str, sizeof(item->hex_str), "%02x", code[offset]);
        strncpy(item->mnemonic, "byte", MAX_MNEMONIC_LEN - 1);
        strncpy(item->cmd, "BYTE", MAX_CMD_LEN - 1);
        snprintf(item->text, sizeof(item->text), ".byte 0x%02x", code[offset]);

        item->type          = NORMAL;
        item->condition     = COND_NONE;
        item->dest_reg      = REG_INVALID;
        item->src_reg       = REG_INVALID;
        item->mem_dest.base = REG_INVALID;
        item->mem_dest.index= REG_INVALID;
        item->mem_src.base  = REG_INVALID;
        item->mem_src.index = REG_INVALID;

        item->n_operands = 1;
        item->operands[0].type = OP_IMMEDIATE;
        item->operands[0].width = WIDTH_8_BIT;
        item->operands[0].role = ROLE_SRC;
        item->operands[0].immediate = (int32_t)code[offset];
        item->immediate_val = (int32_t)code[offset];
        item->has_immediate = true;

        offset += 1;
    }

    return result;
}

/* ========================================================================= */
/* [WORKLET 3 - FRONTEND] Disassemble Directly From 32-bit ELF File          */
/* ========================================================================= */

DisasmResult* disasm_elf_file(const char *filepath) {
    if (!filepath) return NULL;
    elf32_section_t sec;
    if (elf32_load_text_section(filepath, &sec) != 0) {
        return NULL;
    }
    DisasmResult *result = disasm_to_adapter(sec.data, (unsigned int)sec.size, sec.vaddr);
    elf32_free_section(&sec);
    return result;
}

/* ========================================================================= */
/* [WORKLET 3 - FRONTEND] Free DisasmResult Resources                        */
/* ========================================================================= */

void free_adapter_result(DisasmResult *result) {
    if (result) {
        if (result->instructions) {
            free(result->instructions);
            result->instructions = NULL;
        }
        free(result);
    }
}

/* ========================================================================= */
/* [WORKLET 2 - EXECUTION ENGINE] Disassemble ELF into decoded_program       */
/* ========================================================================= */

decoded_program* disasm_for_worklet2(const char *filepath) {
    if (!filepath) return NULL;

    elf32_section_t sec;
    if (elf32_load_text_section(filepath, &sec) != 0) {
        return NULL;
    }

    DisasmResult *dis = disasm_to_adapter(sec.data, (unsigned int)sec.size, sec.vaddr);
    if (!dis) {
        elf32_free_section(&sec);
        return NULL;
    }

    decoded_program *prog = malloc(sizeof(decoded_program));
    if (!prog) {
        free_adapter_result(dis);
        elf32_free_section(&sec);
        return NULL;
    }

    prog->load_base   = sec.vaddr;
    prog->entry_point = sec.entry_point;
    prog->code        = sec.data;        // prog takes ownership of .text bytes
    prog->code_size   = sec.size;
    prog->insn_count  = (int)dis->count;

    prog->insns = malloc(sizeof(worklet2_instruction) * (prog->insn_count > 0 ? prog->insn_count : 1));
    if (!prog->insns) {
        free(prog);
        free_adapter_result(dis);
        elf32_free_section(&sec);
        return NULL;
    }

    for (int i = 0; i < prog->insn_count; i++) {
        Instruction *src = &dis->instructions[i];
        worklet2_instruction *dst = &prog->insns[i];
        memset(dst, 0, sizeof(*dst));

        dst->address = src->start_address;
        dst->length  = (uint8_t)src->length;
        strncpy(dst->mnemonic, src->mnemonic, sizeof(dst->mnemonic) - 1);
        dst->flow    = (uint8_t)src->type;
        dst->target  = src->has_target_address ? src->target_address : 0;

        // Map up to 2 operands
        for (int op = 0; op < 2 && op < (int)src->n_operands; op++) {
            if (src->operands[op].type == OP_REGISTER) {
                dst->operand_kind[op] = W2_OP_REG;
                dst->operand_reg[op]  = (uint8_t)src->operands[op].reg;
            } else if (src->operands[op].type == OP_IMMEDIATE) {
                dst->operand_kind[op] = W2_OP_IMM;
                dst->operand_reg[op]  = 0xFF;
                dst->immediate        = src->operands[op].immediate;
            } else if (src->operands[op].type == OP_MEMORY) {
                dst->operand_kind[op] = W2_OP_MEM;
                dst->operand_reg[op]  = (src->operands[op].mem.base != REG_INVALID) ?
                                         (uint8_t)src->operands[op].mem.base : 0xFF;
            } else {
                dst->operand_kind[op] = W2_OP_NONE;
                dst->operand_reg[op]  = 0xFF;
            }
        }

        if (src->has_immediate && dst->immediate == 0) {
            dst->immediate = src->immediate_val;
        }
    }

    free_adapter_result(dis);
    // Note: Do not call elf32_free_section(&sec) because sec.data ownership moved to prog->code
    return prog;
}

/* ========================================================================= */
/* [WORKLET 2 - EXECUTION ENGINE] Free decoded_program Resources             */
/* ========================================================================= */

void free_decoded_program(decoded_program *prog) {
    if (prog) {
        if (prog->insns) {
            free(prog->insns);
            prog->insns = NULL;
        }
        if (prog->code) {
            free((void *)prog->code);
            prog->code = NULL;
        }
        free(prog);
    }
}
