#include "formatter.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

static const char *get_reg_name(register_t reg) {
    switch (reg) {
        // 8-bit registers
        case AL: return "%al";
        case CL: return "%cl";
        case DL: return "%dl";
        case BL: return "%bl";
        case AH: return "%ah";
        case CH: return "%ch";
        case DH: return "%dh";
        case BH: return "%bh";

        // 16-bit registers
        case AX: return "%ax";
        case CX: return "%cx";
        case DX: return "%dx";
        case BX: return "%bx";
        case SP: return "%sp";
        case BP: return "%bp";
        case SI: return "%si";
        case DI: return "%di";

        // 32-bit registers
        case EAX: return "%eax";
        case ECX: return "%ecx";
        case EDX: return "%edx";
        case EBX: return "%ebx";
        case ESP: return "%esp";
        case EBP: return "%ebp";
        case ESI: return "%esi";
        case EDI: return "%edi";

        default: return "%bad_reg";
    }
}

static const char *get_segment_prefix(const decoded_binary *bin) {
    if (!bin || bin->prefixes.is_valid != YES) {
        return "";
    }
    for (int i = 0; i < bin->prefixes.no_of_prefix; i++) {
        switch (bin->prefixes.prefix_arr[i]) {
            case 0x2E: return "%cs:";
            case 0x36: return "%ss:";
            case 0x3E: return "%ds:";
            case 0x26: return "%es:";
            case 0x64: return "%fs:";
            case 0x65: return "%gs:";
            default: break;
        }
    }
    return "";
}

static const char *get_instruction_prefix(const decoded_binary *bin, const char *mnemonic) {
    if (!bin || bin->prefixes.is_valid != YES) {
        return "";
    }
    for (int i = 0; i < bin->prefixes.no_of_prefix; i++) {
        switch (bin->prefixes.prefix_arr[i]) {
            case 0xF0: return "lock ";
            case 0xF2: return "repne ";
            case 0xF3:
                if (mnemonic && (strcasecmp(mnemonic, "ret") == 0 || strcasecmp(mnemonic, "repz") == 0)) {
                    return "repz ";
                }
                return "rep ";
            default: break;
        }
    }
    return "";
}

static void format_memory_operand(const decoded_binary *bin, const memory_t *mem, char *buf, size_t max_len) {
    const char *seg = get_segment_prefix(bin);

    if (mem->base == REG_NONE && mem->index == REG_NONE) {
        // Pure absolute displacement: 0x8048000
        snprintf(buf, max_len, "%s0x%x", seg, (uint32_t)mem->displacement);
        return;
    }

    if (mem->base != REG_NONE && mem->index == REG_NONE) {
        // Simple base + displacement: disp(%base)
        const char *base_str = get_reg_name(mem->base);
        if (mem->displacement == 0) {
            snprintf(buf, max_len, "%s(%s)", seg, base_str);
        } else if (mem->displacement > 0) {
            snprintf(buf, max_len, "%s0x%x(%s)", seg, mem->displacement, base_str);
        } else {
            snprintf(buf, max_len, "%s-0x%x(%s)", seg, -mem->displacement, base_str);
        }
        return;
    }

    if (mem->base != REG_NONE && mem->index != REG_NONE) {
        // Base + Index + Scale: disp(%base,%index,scale)
        const char *base_str = get_reg_name(mem->base);
        const char *idx_str = get_reg_name(mem->index);

        if (mem->scale == 1) {
            if (mem->displacement == 0) {
                snprintf(buf, max_len, "%s(%s,%s)", seg, base_str, idx_str);
            } else if (mem->displacement > 0) {
                snprintf(buf, max_len, "%s0x%x(%s,%s)", seg, mem->displacement, base_str, idx_str);
            } else {
                snprintf(buf, max_len, "%s-0x%x(%s,%s)", seg, -mem->displacement, base_str, idx_str);
            }
        } else {
            if (mem->displacement == 0) {
                snprintf(buf, max_len, "%s(%s,%s,%u)", seg, base_str, idx_str, mem->scale);
            } else if (mem->displacement > 0) {
                snprintf(buf, max_len, "%s0x%x(%s,%s,%u)", seg, mem->displacement, base_str, idx_str, mem->scale);
            } else {
                snprintf(buf, max_len, "%s-0x%x(%s,%s,%u)", seg, -mem->displacement, base_str, idx_str, mem->scale);
            }
        }
        return;
    }

    if (mem->base == REG_NONE && mem->index != REG_NONE) {
        // Index + Scale without base: disp(,%index,scale)
        const char *idx_str = get_reg_name(mem->index);
        if (mem->scale == 1) {
            snprintf(buf, max_len, "%s0x%x(,%s)", seg, mem->displacement, idx_str);
        } else {
            snprintf(buf, max_len, "%s0x%x(,%s,%u)", seg, mem->displacement, idx_str, mem->scale);
        }
    }
}

static void format_single_operand(const decoded_binary *bin,
                                  const operand_reference_t *op,
                                  char *buf,
                                  size_t max_len) {
    buf[0] = '\0';

    switch (op->encoding_type) {
        case ENCODING_REG:
        case ENCODING_OPCODE_REG:
        case ENCODING_FIXED_REG:
            snprintf(buf, max_len, "%s", get_reg_name(op->operand_value.reg));
            break;

        case ENCODING_RM:
            if (bin->modrm.mod == 3) {
                snprintf(buf, max_len, "%s", get_reg_name(op->operand_value.reg));
            } else {
                format_memory_operand(bin, &op->operand_value.mem_val, buf, max_len);
            }
            break;

        case ENCODING_IMMEDIATE:
            if (op->operand_value.imm_val.extension == EXTENSION_SIGN) {
                snprintf(buf, max_len, "$0x%x", op->operand_value.imm_val.value);
            } else if (op->operand_width == WIDTH_8) {
                snprintf(buf, max_len, "$0x%x", (uint8_t)op->operand_value.imm_val.value);
            } else if (op->operand_width == WIDTH_16) {
                snprintf(buf, max_len, "$0x%x", (uint16_t)op->operand_value.imm_val.value);
            } else {
                snprintf(buf, max_len, "$0x%x", op->operand_value.imm_val.value);
            }
            break;

        case ENCODING_RELATIVE:
            // Branches/Calls in objdump AT&T syntax print target address directly in hex
            snprintf(buf, max_len, "0x%x", (uint32_t)op->operand_value.rel_val);
            break;

        case ENCODING_NONE:
        case ENCODING_IMPLICIT:
        default:
            break;
    }
}

int format_assembly_only(const decoded_binary *bin,
                         const instruction_reference_t *inst,
                         char *buffer,
                         size_t max_len) {
    if (!bin || !inst || !buffer || max_len == 0) {
        return -1;
    }

    // Convert mnemonic to lowercase for standard AT&T output
    char mnemonic_lower[32];
    size_t i;
    for (i = 0; inst->mnemonic_name && inst->mnemonic_name[i] && i < sizeof(mnemonic_lower) - 1; i++) {
        mnemonic_lower[i] = (char)tolower((unsigned char)inst->mnemonic_name[i]);
    }
    mnemonic_lower[i] = '\0';

    const char *prefix_str = get_instruction_prefix(bin, inst->mnemonic_name);
    char full_mnemonic[48];
    snprintf(full_mnemonic, sizeof(full_mnemonic), "%s%s", prefix_str, mnemonic_lower);

    // Format operands
    if (inst->operand_count == COUNT_ZERO) {
        snprintf(buffer, max_len, "%s", full_mnemonic);
        return (int)strlen(buffer);
    }

    if (inst->operand_count == COUNT_ONE) {
        char op0[64];
        format_single_operand(bin, &inst->operands[0], op0, sizeof(op0));
        snprintf(buffer, max_len, "%-6s %s", full_mnemonic, op0);
        return (int)strlen(buffer);
    }

    if (inst->operand_count == COUNT_TWO) {
        // AT&T Syntax order is: SOURCE, DESTINATION
        char op_src[64];
        char op_dst[64];

        format_single_operand(bin, &inst->operands[1], op_src, sizeof(op_src));
        format_single_operand(bin, &inst->operands[0], op_dst, sizeof(op_dst));

        snprintf(buffer, max_len, "%-6s %s,%s", full_mnemonic, op_src, op_dst);
        return (int)strlen(buffer);
    }

    if (inst->operand_count == COUNT_THREE) {
        // E.g. IMUL reg, rm, imm -> AT&T: imul $imm, %rm, %reg
        char op0[64], op1[64], op2[64];
        format_single_operand(bin, &inst->operands[2], op2, sizeof(op2));
        format_single_operand(bin, &inst->operands[1], op1, sizeof(op1));
        format_single_operand(bin, &inst->operands[0], op0, sizeof(op0));

        snprintf(buffer, max_len, "%-6s %s,%s,%s", full_mnemonic, op2, op1, op0);
        return (int)strlen(buffer);
    }

    snprintf(buffer, max_len, "%s", full_mnemonic);
    return (int)strlen(buffer);
}

int format_instruction_att(const decoded_binary *bin,
                           const instruction_reference_t *inst,
                           uint32_t addr,
                           char *buffer,
                           size_t max_len) {
    if (!bin || !inst || !buffer || max_len == 0) {
        return -1;
    }

    // Format raw bytes as hex string (e.g. "55 " or "89 e5 ")
    char bytes_str[64] = {0};
    size_t offset = 0;
    for (int j = 0; j < bin->length && offset < sizeof(bytes_str) - 4; j++) {
        offset += (size_t)snprintf(bytes_str + offset, sizeof(bytes_str) - offset, "%02x ", bin->raw_bytes[j]);
    }

    // Format assembly instruction
    char asm_str[128] = {0};
    format_assembly_only(bin, inst, asm_str, sizeof(asm_str));

    // Combine into standard objdump -d tabular layout:
    // "    1000:\t53                  \tpush   %ebx"
    snprintf(buffer, max_len, "    %4x:\t%-20s\t%s", addr, bytes_str, asm_str);
    return (int)strlen(buffer);
}
