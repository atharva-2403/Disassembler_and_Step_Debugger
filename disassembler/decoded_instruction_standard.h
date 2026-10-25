#include "prefix_standard.h"

#define MAX_PREFIX_LIMIT 4

typedef enum{
	SEG_DEFAULT = 0x00,
	SEG_ES = 0x26,
	SEG_CS = 0x2E,
	SEG_SS = 0x36,
	SEG_DS = 0x3E,
	SEG_FS = 0x64,
	SEG_GS = 0x65
}segment_t;

typedef struct{
	PrefixInfo prefixes[MAX_PREFIX_LIMIT];
	int prefix_count;

	bool lock_enabled;
	bool repnz_enabled;
	bool rep_enabled;
	bool seg_override_enabled;
	bool opsize_override_enabled;
	bool addrsize_override_enabled;
	
	segment_t segment_reg;


}decoded_instruction_definition;
