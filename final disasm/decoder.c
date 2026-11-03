#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "decoder.h"
#include "prefix.h"

// sizes of the immediate value used in the tables below
//   I0 = no immediate
//   I1 = 1 byte
//   I2 = 2 bytes
//   IZ = 4 bytes, but 2 bytes if the 0x66 prefix is there
//   IM = always 4 bytes (like mov eax,[address])
//   IX = opcode not supported by this decoder
#define I0 0
#define I1 1
#define I2 2
#define IZ 4
#define IM 5
#define IX 9

// ---------------------------------------------------------------
// one byte opcodes
// each row is the high nibble and each column is the low nibble
// ---------------------------------------------------------------

// 1 means the opcode has a modrm byte after it
static const uint8_t one_modrm[256] = {
    1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 0x00
    1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 0x10
    1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 0x20
    1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, // 0x30
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x40
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x50
    0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, // 0x60
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x70
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x80
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x90
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xA0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xB0
    1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, // 0xC0
    1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, // 0xD0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xE0
    0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, // 0xF0
};

// size of the immediate after the opcode (I0 to IX from the top)
static const uint8_t one_imm[256] = {
    I0, I0, I0, I0, I1, IZ, I0, I0, I0, I0, I0, I0, I1, IZ, I0, I0, // 0x00
    I0, I0, I0, I0, I1, IZ, I0, I0, I0, I0, I0, I0, I1, IZ, I0, I0, // 0x10
    I0, I0, I0, I0, I1, IZ, I0, I0, I0, I0, I0, I0, I1, IZ, I0, I0, // 0x20
    I0, I0, I0, I0, I1, IZ, I0, I0, I0, I0, I0, I0, I1, IZ, I0, I0, // 0x30
    I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, // 0x40
    I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, // 0x50
    I0, I0, I0, I0, I0, I0, I0, I0, IZ, IZ, I1, I1, I0, I0, I0, I0, // 0x60
    I1, I1, I1, I1, I1, I1, I1, I1, I1, I1, I1, I1, I1, I1, I1, I1, // 0x70
    I1, IZ, I1, I1, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, // 0x80
    I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, IX, I0, I0, I0, I0, I0, // 0x90
    IM, IM, IM, IM, I0, I0, I0, I0, I1, IZ, I0, I0, I0, I0, I0, I0, // 0xA0
    I1, I1, I1, I1, I1, I1, I1, I1, IZ, IZ, IZ, IZ, IZ, IZ, IZ, IZ, // 0xB0
    I1, I1, I2, I0, I0, I0, I1, IZ, IX, I0, I2, I0, I0, I1, I0, I0, // 0xC0
    I0, I0, I0, I0, I1, I1, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, // 0xD0
    I1, I1, I1, I1, I1, I1, I1, I1, IZ, IZ, IX, I1, I0, I0, I0, I0, // 0xE0
    I0, I0, I0, I0, I0, I0, I1, IZ, I0, I0, I0, I0, I0, I0, I0, I0, // 0xF0
};

// ---------------------------------------------------------------
// two byte opcodes (0x0F xx)
// only the common ones are filled in, the rest are IX (not supported)
// ---------------------------------------------------------------

static const uint8_t two_modrm[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, // 0x10
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x20
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x30
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x50
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x60
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x70
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x80
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x90
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, // 0xA0
    0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, // 0xB0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xC0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xD0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xE0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xF0
};

static const uint8_t two_imm[256] = {
    IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, I0, IX, IX, IX, IX, // 0x00
    IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, I0, I0, // 0x10
    IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, // 0x20
    IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, // 0x30
    I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, // 0x40
    IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, // 0x50
    IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, // 0x60
    IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, // 0x70
    IZ, IZ, IZ, IZ, IZ, IZ, IZ, IZ, IZ, IZ, IZ, IZ, IZ, IZ, IZ, IZ, // 0x80
    I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, I0, // 0x90
    IX, IX, I0, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, I0, // 0xA0
    IX, IX, IX, IX, IX, IX, I0, I0, IX, IX, IX, IX, IX, IX, I0, I0, // 0xB0
    IX, IX, IX, IX, IX, IX, IX, IX, I0, I0, I0, I0, I0, I0, I0, I0, // 0xC0
    IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, // 0xD0
    IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, // 0xE0
    IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, IX, // 0xF0
};

// number of bytes -> enum value (the index is the number of bytes)
static const imm_size_t imm_enum[5] = {IMM_NONE, IMM_8, IMM_16, IMM_NONE,
                                       IMM_32};
static const disp_size_t disp_enum[5] = {DISP0, DISP8, DISP0, DISP0, DISP32};

// number of displacement bytes for mod = 0, 1, 2, 3
static const uint8_t disp_by_mod[4] = {0, 1, 4, 0};

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

// set everything in the struct to "not present"
void init_decoded(decoded_binary *d) {
  memset(d, 0, sizeof(decoded_binary));
  d->prefixes.is_valid = NO;
  d->opcode.is_valid = NO;
  d->modrm.is_valid = NO;
  d->sib.is_valid = NO;
  d->disp.is_valid = NO;
  d->disp.size = DISP0;
  d->immediate.is_valid = NO;
  d->immediate.size = IMM_NONE;
}

// keep reading bytes as long as they are prefixes
int decode_prefixes(uint8_t *bytes, int pos, int size, decoded_binary *d) {
  while (pos < size && get_prefix(bytes[pos]).is_prefix) {
    if (d->prefixes.no_of_prefix >= MAX_PREFIX_LIMIT) {
      return -1;
    }
    d->prefixes.prefix_arr[d->prefixes.no_of_prefix] = bytes[pos];
    d->prefixes.no_of_prefix++;
    pos++;
  }
  if (d->prefixes.no_of_prefix > 0) {
    d->prefixes.is_valid = YES;
  }
  return pos;
}

// read the opcode (1 byte, or 2 bytes if the first one is 0x0F)
int decode_opcode(uint8_t *bytes, int pos, int size, decoded_binary *d) {
  if (pos >= size) {
    return -1;
  }
  d->opcode.bytes[0] = bytes[pos];
  d->opcode.size = ONE_B_OPCODE;
  pos++;

  if (d->opcode.bytes[0] == 0x0F) {
    if (pos >= size) {
      return -1;
    }
    d->opcode.bytes[1] = bytes[pos];
    d->opcode.size = TWO_B_OPCODE;
    pos++;
    if (two_imm[d->opcode.bytes[1]] == IX) {
      return -1; // not supported
    }
  } else {
    if (one_imm[d->opcode.bytes[0]] == IX) {
      return -1; // not supported
    }
  }

  d->opcode.is_valid = YES;
  return pos;
}

// check the table to see if this opcode has a modrm byte
int needs_modrm(decoded_binary *d) {
  if (d->opcode.size == ONE_B_OPCODE) {
    return one_modrm[d->opcode.bytes[0]];
  }
  return two_modrm[d->opcode.bytes[1]];
}

// split the modrm byte into mod, reg and rm
int decode_modrm(uint8_t *bytes, int pos, int size, decoded_binary *d) {
  if (pos >= size) {
    return -1;
  }
  d->modrm.mod = (bytes[pos] >> 6) & 0x3;
  d->modrm.reg = (bytes[pos] >> 3) & 0x7;
  d->modrm.rm = bytes[pos] & 0x7;
  d->modrm.is_valid = YES;
  return pos + 1;
}

// sib byte is there when mod is not 3 and rm is 4 (never in 16-bit addressing)
int needs_sib(decoded_binary *d) {
  if (has_prefix_67(d)) {
    return 0;
  }
  if (d->modrm.mod != 3 && d->modrm.rm == 4) {
    return 1;
  }
  return 0;
}

// split the sib byte into scale, index and base
int decode_sib(uint8_t *bytes, int pos, int size, decoded_binary *d) {
  if (pos >= size) {
    return -1;
  }
  d->sib.scale = (bytes[pos] >> 6) & 0x3;
  d->sib.index = (bytes[pos] >> 3) & 0x7;
  d->sib.base = bytes[pos] & 0x7;
  d->sib.is_valid = YES;
  return pos + 1;
}

// displacement size depends on mod, address size, and special cases
int decode_disp(uint8_t *bytes, int pos, int size, decoded_binary *d) {
  int count;
  int i;

  if (has_prefix_67(d)) {
    if (d->modrm.mod == 0) {
      count = (d->modrm.rm == 6) ? 2 : 0;
    } else if (d->modrm.mod == 1) {
      count = 1;
    } else if (d->modrm.mod == 2) {
      count = 2;
    } else {
      count = 0;
    }
  } else {
    count = disp_by_mod[d->modrm.mod];

    if (d->modrm.mod == 0 && d->modrm.rm == 5) {
      count = 4; // [disp32]
    }
    if (d->modrm.mod == 0 && d->sib.is_valid == YES && d->sib.base == 5) {
      count = 4; // [index*scale + disp32]
    }
  }

  if (count == 0) {
    return pos;
  }
  if (pos + count > size) {
    return -1;
  }

  for (i = 0; i < count; i++) {
    d->disp.bytes[i] = bytes[pos + i];
  }
  d->disp.size = disp_enum[count];
  d->disp.is_valid = YES;
  return pos + count;
}

// immediate size comes from the table
int decode_immediate(uint8_t *bytes, int pos, int size, decoded_binary *d) {
  int count;
  int i;

  if (d->opcode.size == ONE_B_OPCODE) {
    count = one_imm[d->opcode.bytes[0]];
  } else {
    count = two_imm[d->opcode.bytes[1]];
  }

  if (count == IM) {
    count = 4; // always 4 bytes, 0x66 prefix does not change it
  } else if (count == IZ) {
    count = 4;
    for (i = 0; i < d->prefixes.no_of_prefix; i++) {
      if (d->prefixes.prefix_arr[i] == 0x66) {
        count = 2; // 0x66 prefix makes it 16 bit
      }
    }
  }

  // F6 and F7 only have an immediate when reg is 0 or 1 (TEST)
  if ((d->opcode.bytes[0] == 0xF6 || d->opcode.bytes[0] == 0xF7) &&
      d->modrm.reg > 1) {
    count = 0;
  }

  if (count == 0) {
    return pos;
  }
  if (pos + count > size) {
    return -1;
  }

  for (i = 0; i < count; i++) {
    d->immediate.bytes[i] = bytes[pos + i];
  }
  d->immediate.size = imm_enum[count];
  d->immediate.is_valid = YES;
  return pos + count;
}

// decode one instruction, returns its length (0 if it cannot be decoded)
int decode_instruction(uint8_t *bytes, int pos, int size, decoded_binary *d) {
  int start;
  int i;

  start = pos;
  init_decoded(d);

  pos = decode_prefixes(bytes, pos, size, d);
  if (pos < 0) {
    return 0;
  }

  pos = decode_opcode(bytes, pos, size, d);
  if (pos < 0) {
    return 0;
  }

  if (needs_modrm(d)) {
    pos = decode_modrm(bytes, pos, size, d);
    if (pos < 0) {
      return 0;
    }

    if (needs_sib(d)) {
      pos = decode_sib(bytes, pos, size, d);
      if (pos < 0) {
        return 0;
      }
    }

    pos = decode_disp(bytes, pos, size, d);
    if (pos < 0) {
      return 0;
    }
  }

  pos = decode_immediate(bytes, pos, size, d);
  if (pos < 0) {
    return 0;
  }

  d->length = pos - start;
  if (d->length > MAX_INSTRUCTION_LENGTH) {
    return 0;
  }

  for (i = 0; i < d->length; i++) {
    d->raw_bytes[i] = bytes[start + i];
  }
  return d->length;
}

// print every part of the decoded instruction
void print_decoded(decoded_binary *d) {
  int i;
  int count;

  printf("bytes  :");
  for (i = 0; i < d->length; i++) {
    printf(" %02X", d->raw_bytes[i]);
  }
  printf("   (length %d)\n", d->length);

  if (d->prefixes.is_valid == YES) {
    printf("prefix :");
    for (i = 0; i < d->prefixes.no_of_prefix; i++) {
      printf(" %02X", d->prefixes.prefix_arr[i]);
    }
    printf("\n");
  }

  printf("opcode :");
  for (i = 0; i < (int)d->opcode.size; i++) {
    printf(" %02X", d->opcode.bytes[i]);
  }
  printf("\n");

  if (d->modrm.is_valid == YES) {
    printf("modrm  : mod=%d reg=%d rm=%d\n", d->modrm.mod, d->modrm.reg,
           d->modrm.rm);
  }

  if (d->sib.is_valid == YES) {
    printf("sib    : scale=%d index=%d base=%d\n", d->sib.scale, d->sib.index,
           d->sib.base);
  }

  if (d->disp.is_valid == YES) {
    printf("disp   :");
    if (d->disp.size == DISP8) {
      count = 1;
    } else {
      count = 4;
    }
    for (i = 0; i < count; i++) {
      printf(" %02X", d->disp.bytes[i]);
    }
    printf("\n");
  }

  if (d->immediate.is_valid == YES) {
    printf("imm    :");
    if (d->immediate.size == IMM_8) {
      count = 1;
    } else if (d->immediate.size == IMM_16) {
      count = 2;
    } else {
      count = 4;
    }
    for (i = 0; i < count; i++) {
      printf(" %02X", d->immediate.bytes[i]);
    }
    printf("\n");
  }
}
