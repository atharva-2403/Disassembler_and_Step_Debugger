#ifndef PREFIX_FRAMEWORK_H
#define PREFIX_FRAMEWORK_H

#include <stdint.h>
#include <stdbool.h>

// Define prefix groups or types
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
    ACTION_REP,       // For 0xF3
    ACTION_REPNZ,     // For 0xF2
    ACTION_SEG_OVERRIDE,
    ACTION_OPSIZE,    // For 0x66
    ACTION_ADDRSIZE   // For 0x67
} PrefixAction;

// Structure to hold information about a prefix byte
typedef struct {
    bool is_prefix;
    PrefixType type;
    const char* name;
    PrefixAction action;
} PrefixInfo;

#endif
