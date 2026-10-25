#include "prefix_standard.h"

static const PrefixInfo prefix_table[256] = {
    [0 ... 255] = { 
        .is_prefix = false,
        .type = PREFIX_NONE,   // Fixed from .type
        .name = "NULL",     // Fixed from .NAME
        .action = ACTION_NONE 
    },
    // Group 1
    [0xF0] = { true, GROUP_1_LOCK_REP, "LOCK",  ACTION_LOCK },
    [0xF2] = { true, GROUP_1_LOCK_REP, "REPNZ", ACTION_REPNZ },
    [0xF3] = { true, GROUP_1_LOCK_REP, "REP",   ACTION_REP },

    // Group 2: Segment Overrides
    [0x26] = { true, GROUP_2_SEGMENT, "ES:", ACTION_SEG_OVERRIDE },
    [0x2E] = { true, GROUP_2_SEGMENT, "CS:", ACTION_SEG_OVERRIDE },
    [0x36] = { true, GROUP_2_SEGMENT, "SS:", ACTION_SEG_OVERRIDE },
    [0x3E] = { true, GROUP_2_SEGMENT, "DS:", ACTION_SEG_OVERRIDE },
    [0x64] = { true, GROUP_2_SEGMENT, "FS:", ACTION_SEG_OVERRIDE },
    [0x65] = { true, GROUP_2_SEGMENT, "GS:", ACTION_SEG_OVERRIDE },

    // Group 3 & 4
    [0x66] = { true, GROUP_3_OPSIZE,  "OPSIZE",   ACTION_OPSIZE },
    [0x67] = { true, GROUP_4_ADDRSIZE, "ADDRSIZE", ACTION_ADDRSIZE }
};
