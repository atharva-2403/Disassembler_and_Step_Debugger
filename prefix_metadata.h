#ifndef PREFIX_METADATA_H
#define PREFIX_METADATA_H

#include <stdint.h>
#include <stdbool.h>

// x86 prefixes fall into 4 groups:
//   Group 1 : LOCK / REP / REPNZ  — affect instruction behaviour
//   Group 2 : Segment overrides    — change default segment register
//   Group 3 : OPSIZE (0x66)        — toggles between 32-bit and 16-bit operands
//   Group 4 : ADDRSIZE (0x67)      — toggles address calculation size
typedef enum {
    PREFIX_NONE = 0,
    GROUP_1_LOCK_REP,
    GROUP_2_SEGMENT,
    GROUP_3_OPSIZE,
    GROUP_4_ADDRSIZE
} PrefixType;

typedef enum {
    ACTION_NONE = 0,
    ACTION_LOCK,
    ACTION_REP,
    ACTION_REPNZ,
    ACTION_SEG_OVERRIDE,
    ACTION_OPSIZE,
    ACTION_ADDRSIZE
} PrefixAction;

// One entry in the prefix lookup table — indexed directly by byte value.
typedef struct {
    bool        is_prefix;  // true if this byte is an x86 prefix
    PrefixType  type;
    const char *name;       // human-readable label (e.g. "LOCK", "OPSIZE")
    PrefixAction action;
} PrefixInfo;

// 256-slot table: index with any byte to instantly check if it is a prefix.
// Unspecified entries are zero-initialised (is_prefix = false).
static const PrefixInfo prefix_table[256] = {
    // Group 1: LOCK / REP / REPNZ
    [0xF0] = { true, GROUP_1_LOCK_REP, "LOCK",  ACTION_LOCK  },
    [0xF2] = { true, GROUP_1_LOCK_REP, "REPNZ", ACTION_REPNZ },
    [0xF3] = { true, GROUP_1_LOCK_REP, "REP",   ACTION_REP   },

    // Group 2: Segment overrides
    [0x26] = { true, GROUP_2_SEGMENT, "ES:", ACTION_SEG_OVERRIDE },
    [0x2E] = { true, GROUP_2_SEGMENT, "CS:", ACTION_SEG_OVERRIDE },
    [0x36] = { true, GROUP_2_SEGMENT, "SS:", ACTION_SEG_OVERRIDE },
    [0x3E] = { true, GROUP_2_SEGMENT, "DS:", ACTION_SEG_OVERRIDE },
    [0x64] = { true, GROUP_2_SEGMENT, "FS:", ACTION_SEG_OVERRIDE },
    [0x65] = { true, GROUP_2_SEGMENT, "GS:", ACTION_SEG_OVERRIDE },

    // Group 3: Operand-size override
    [0x66] = { true, GROUP_3_OPSIZE,   "OPSIZE",   ACTION_OPSIZE   },

    // Group 4: Address-size override
    [0x67] = { true, GROUP_4_ADDRSIZE, "ADDRSIZE", ACTION_ADDRSIZE }
};

#endif
