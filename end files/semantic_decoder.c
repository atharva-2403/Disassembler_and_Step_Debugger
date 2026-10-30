#include <string.h>

#include "instruction_table.h"
#include "semantic_decoder.h"

// register index -> register enum
static const register_t reg_table_8[8] = {AL, CL, DL, BL, AH, CH, DH, BH};
static const register_t reg_table_16[8] = {AX, CX, DX, BX, SP, BP, SI, DI};
static const register_t reg_table_32[8] = {EAX, ECX, EDX, EBX,
                                           ESP, EBP, ESI, EDI};

// check if 0x66 prefix is present
static int has_prefix_66(const decoded_binary *d) {
  int i;

  if (d->prefixes.is_valid != YES) {
    return 0;
  }
  for (i = 0; i < d->prefixes.no_of_prefix; i++) {
    if (d->prefixes.prefix_arr[i] == 0x66) {
      return 1;
    }
  }
  return 0;
}

// check if 0x67 prefix is present
static int has_prefix_67(const decoded_binary *d) {
  int i;

  if (d->prefixes.is_valid != YES) {
    return 0;
  }
  for (i = 0; i < d->prefixes.no_of_prefix; i++) {
    if (d->prefixes.prefix_arr[i] == 0x67) {
      return 1;
    }
  }
  return 0;
}

// 3-bit index -> register based on width and 0x66 prefix
static register_t resolve_register(uint8_t index, operand_width_t width,
                                   int opsize_prefix) {
  index &= 0x07;

  if (width == WIDTH_8) {
    return reg_table_8[index];
  }

  // 0x66 prefix: 32-bit -> 16-bit
  if (width == WIDTH_16 || (width == WIDTH_32 && opsize_prefix)) {
    return reg_table_16[index];
  }

  return reg_table_32[index];
}

// resolve memory operand
static memory_t resolve_memory(const decoded_binary *d) {
  memory_t mem;

  mem.base = REG_NONE;
  mem.index = REG_NONE;
  mem.scale = 1;
  mem.displacement = 0;

  if (has_prefix_67(d)) {
    if (d->disp.is_valid == YES) {
      if (d->disp.size == DISP8) {
        mem.displacement = (int8_t)d->disp.bytes[0];
      } else {
        mem.displacement = (int16_t)(d->disp.bytes[0] | (d->disp.bytes[1] << 8));
      }
    }

    switch (d->modrm.rm) {
      case 0: mem.base = BX; mem.index = SI; break;
      case 1: mem.base = BX; mem.index = DI; break;
      case 2: mem.base = BP; mem.index = SI; break;
      case 3: mem.base = BP; mem.index = DI; break;
      case 4: mem.base = SI; break;
      case 5: mem.base = DI; break;
      case 6:
        if (d->modrm.mod == 0) {
          mem.base = REG_NONE;
        } else {
          mem.base = BP;
        }
        break;
      case 7: mem.base = BX; break;
    }
    return mem;
  }

  if (d->disp.is_valid == YES) {
    if (d->disp.size == DISP8) {
      mem.displacement = (int8_t)d->disp.bytes[0]; // sign extend
    } else if (d->disp.size == DISP32) {
      mem.displacement = (int32_t)(d->disp.bytes[0] |
                                   (d->disp.bytes[1] << 8) |
                                   (d->disp.bytes[2] << 16) |
                                   ((uint32_t)d->disp.bytes[3] << 24));
    }
  }

  if (d->sib.is_valid == YES) {
    mem.scale = 1 << d->sib.scale;

    // index 4 -> no index register (esp cannot be index)
    if (d->sib.index != 4) {
      mem.index = reg_table_32[d->sib.index];
    }

    // mod 0 and base 5 -> [disp32] with no base
    if (d->modrm.mod == 0 && d->sib.base == 5) {
      mem.base = REG_NONE;
    } else {
      mem.base = reg_table_32[d->sib.base];
    }
  } else {
    // no sib -> base is in rm
    // mod 0 and rm 5 -> [disp32] with no base
    if (d->modrm.mod == 0 && d->modrm.rm == 5) {
      mem.base = REG_NONE;
    } else {
      mem.base = reg_table_32[d->modrm.rm];
    }
  }

  return mem;
}

// resolve immediate value
static immediate_t resolve_immediate(const decoded_binary *d,
                                     operand_width_t width,
                                     int opsize_prefix) {
  immediate_t imm;
  uint8_t raw;

  imm.value = 0;
  imm.extension = EXTENSION_NONE;

  if (d->immediate.is_valid != YES) {
    return imm;
  }

  if (d->immediate.size == IMM_8) {
    raw = d->immediate.bytes[0];
    // 8-bit imm with 32-bit op -> sign extend to 32 bits
    if (width == WIDTH_32 || d->opcode.bytes[0] == 0x83 ||
        d->opcode.bytes[0] == 0x6A || d->opcode.bytes[0] == 0x6B) {
      imm.value = (int32_t)(int8_t)raw;
      imm.extension = EXTENSION_SIGN;
    } else {
      imm.value = raw;
      imm.extension = EXTENSION_NONE;
    }
  } else if (d->immediate.size == IMM_16) {
    imm.value = d->immediate.bytes[0] | (d->immediate.bytes[1] << 8);
    imm.extension = EXTENSION_NONE;
  } else {
    if (opsize_prefix) {
      // 0x66 prefix -> 16-bit immediate
      imm.value = d->immediate.bytes[0] | (d->immediate.bytes[1] << 8);
    } else {
      imm.value = d->immediate.bytes[0] |
                  (d->immediate.bytes[1] << 8) |
                  (d->immediate.bytes[2] << 16) |
                  ((uint32_t)d->immediate.bytes[3] << 24);
    }
    imm.extension = EXTENSION_NONE;
  }

  return imm;
}

// relative branch target for jmp, jcc and call
static int32_t resolve_relative_target(const decoded_binary *d, uint32_t addr,
                                       operand_width_t width) {
  int32_t rel_offset;

  if (width == WIDTH_8) {
    rel_offset = (int8_t)d->immediate.bytes[0];
  } else {
    rel_offset = (int32_t)(d->immediate.bytes[0] |
                           (d->immediate.bytes[1] << 8) |
                           (d->immediate.bytes[2] << 16) |
                           ((uint32_t)d->immediate.bytes[3] << 24));
  }

  return (int32_t)(addr + d->length + rel_offset);
}

int decode_semantic(const decoded_binary *d, uint32_t addr,instruction_reference_t *inst) {
  const instruction_reference_t *template_entry;
  int opsize_prefix;
  int i;
  operand_reference_t *op;

  if (!d || !inst || d->length == 0) {
    return -1;
  }

  // look up instruction in table
  template_entry = find_instruction_reference(d);
  if (!template_entry) {
    return -1; // not supported
  }

  memcpy(inst, template_entry, sizeof(instruction_reference_t));

  if (d->prefixes.is_valid == YES) {
    inst->prefix_count = d->prefixes.no_of_prefix;
    for (i = 0; i < d->prefixes.no_of_prefix && i < MAX_PREFIXES; i++) {
      inst->prefixes_needed[i] = d->prefixes.prefix_arr[i];
    }
  } else {
    inst->prefix_count = 0;
  }

  opsize_prefix = has_prefix_66(d);

  for (i = 0; i < (int)inst->operand_count; i++) {
    op = &inst->operands[i];

    // 0x66 prefix: 32-bit -> 16-bit
    if (opsize_prefix && op->operand_width == WIDTH_32) {
      op->operand_width = WIDTH_16;
    }

    switch (op->encoding_type) {
    case ENCODING_REG:
      op->operand_value.reg =
          resolve_register(d->modrm.reg, op->operand_width, opsize_prefix);
      break;

    case ENCODING_OPCODE_REG:
      // opcode low 3 bits -> reg
      op->operand_value.reg = resolve_register(
          d->opcode.bytes[0] & 0x07, op->operand_width, opsize_prefix);
      break;

    case ENCODING_FIXED_REG:
      // 0x66 prefix: eax -> ax
      if (opsize_prefix && op->operand_value.reg == EAX) {
        op->operand_value.reg = AX;
      }
      break;

    case ENCODING_RM:
      // mod 3 -> reg, otherwise memory
      if (d->modrm.mod == 3) {
        op->operand_value.reg =
            resolve_register(d->modrm.rm, op->operand_width, opsize_prefix);
      } else {
        op->operand_value.mem_val = resolve_memory(d);
      }
      break;

    case ENCODING_IMMEDIATE:
      op->operand_value.imm_val =
          resolve_immediate(d, op->operand_width, opsize_prefix);
      break;

    case ENCODING_RELATIVE:
      op->operand_value.rel_val =
          resolve_relative_target(d, addr, op->operand_width);
      break;

    default:
      break;
    }
  }

  return 0;
}
