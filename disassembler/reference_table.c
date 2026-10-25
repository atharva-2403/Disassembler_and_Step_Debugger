#include "reference_table_standard.h"
#include <string.h>

static opcode_table_entry_t primary_table[256];
static opcode_table_entry_t secondary_table[256];
static opcode_table_entry_t tertiary_table_38[256];
static opcode_table_entry_t tertiary_table_3A[256];

opcode_table_entry_t *get_opcode_entry(opcode_table_type_t table_type, uint8_t opcode){
	
	switch(table_type){
		case TABLE_PRIMARY:
			return &primary_table[opcode];
		case TABLE_SECONDARY:
			return &secondary_table[opcode];
		case TABLE_TERTIARY_38:
			return &tertiary_table_38[opcode];
		case TABLE_TERTIARY_3A:
			return &tertiary_table_3A[opcode];
		default:
			return NULL;
	}
}

// now initialising the actual instruction fields. 

void init_opcode_tables(void){
	
	// ensuring that all the tables are completely clean before populating them.
	memset(primary_table, 0, sizeof(primary_table));
	memset(secondary_table, 0, sizeof(secondary_table));
	memset(tertiary_table_38, 0, sizeof(tertiary_table_38));
	memset(tertiary_table_3A, 0, sizeof(tertiary_table_3A));
	
    primary_table[0x30] = (opcode_table_entry_t){
        .type = ENTRY_TYPE_SINGLE,
        .content.single_instruction = {
            .mnemonic = "xor",
            .opcode_bytes = { 0x30 },
            .opcode_length = ONE_B_OPCODE,
            .modrm_required = true,
            .operand_count = COUNT_TWO,
            .operands = {
                {
                    .encoding_type = ENCODING_RM,
                    .operand_width = WIDTH_8,
                    .operand_role = OPERAND_SRC_DEST,
                    .opsize_sensitive = false,
                    .addrsize_sensitive = true 
                },
                {
                    .encoding_type = ENCODING_REG,
                    .operand_width = WIDTH_8,
                    .operand_role = OPERAND_SOURCE,
                    .opsize_sensitive = false,
                    .addrsize_sensitive = false
                }
            }
        }
    };

	primary_table[0x31] = (opcode_table_entry_t){
        .type = ENTRY_TYPE_SINGLE,
        .content.single_instruction = {
            .mnemonic = "xor",
            .opcode_bytes = { 0x31 },
            .opcode_length = ONE_B_OPCODE,
            .modrm_required = true,
            .operand_count = COUNT_TWO,
            .operands = {
                {
                    .encoding_type = ENCODING_RM,
                    .operand_width = WIDTH_32,
                    .operand_role = OPERAND_SRC_DEST,
                    .opsize_sensitive = true, 
                    .addrsize_sensitive = true 
                },
                {
                    .encoding_type = ENCODING_REG,
                    .operand_width = WIDTH_32,
                    .operand_role = OPERAND_SOURCE,
                    .opsize_sensitive = true, 
                    .addrsize_sensitive = false
                }
            }
        }
    };

    primary_table[0x32] = (opcode_table_entry_t){
        .type = ENTRY_TYPE_SINGLE,
        .content.single_instruction = {
            .mnemonic = "xor",
            .opcode_bytes = { 0x32 },
            .opcode_length = ONE_B_OPCODE,
            .modrm_required = true,
            .operand_count = COUNT_TWO,
            .operands = {
                {
                    .encoding_type = ENCODING_REG,
                    .operand_width = WIDTH_8,
                    .operand_role = OPERAND_SRC_DEST,
                    .opsize_sensitive = false,
                    .addrsize_sensitive = false
                },
                {
                    .encoding_type = ENCODING_RM,
                    .operand_width = WIDTH_8,
                    .operand_role = OPERAND_SOURCE,
                    .opsize_sensitive = false,
                    .addrsize_sensitive = true   
                }
            }
        }
    };

	primary_table[0x33] = (opcode_table_entry_t){
        .type = ENTRY_TYPE_SINGLE,
        .content.single_instruction = {
            .mnemonic = "xor",
            .opcode_bytes = { 0x33 },
            .opcode_length = ONE_B_OPCODE,
            .modrm_required = true,
            .operand_count = COUNT_TWO,
            .operands = {
                {
                    .encoding_type = ENCODING_REG,
                    .operand_width = WIDTH_32,
                    .operand_role = OPERAND_SRC_DEST,
                    .opsize_sensitive = true,          
                    .addrsize_sensitive = false
                },
                {
                    .encoding_type = ENCODING_RM,
                    .operand_width = WIDTH_32,
                    .operand_role = OPERAND_SOURCE,   
                    .opsize_sensitive = true,          
                    .addrsize_sensitive = true        
                }
            }
        }
    };

	primary_table[0x34] = (opcode_table_entry_t){
        .type = ENTRY_TYPE_SINGLE,
        .content.single_instruction = {
            .mnemonic = "xor",
            .opcode_bytes = { 0x34 },
            .opcode_length = ONE_B_OPCODE,
            .modrm_required = false,
            .operand_count = COUNT_TWO,
            .operands = {
                {
                    .encoding_type = ENCODING_FIXED_REG,
                    .operand_width = WIDTH_8,
                    .operand_value = { .reg = AL },
                    .operand_role = OPERAND_SRC_DEST,
                    .opsize_sensitive = false,
                    .addrsize_sensitive = false
                },
                {
                    .encoding_type = ENCODING_IMMEDIATE,
                    .operand_width = WIDTH_8,
                    .operand_value = { .imm_val = { .value = 0, .extension = EXTENSION_SIGN } },
                    .operand_role = OPERAND_SOURCE,
                    .opsize_sensitive = false,
                    .addrsize_sensitive = false
                }
            }
        }
    };

	primary_table[0x35] = (opcode_table_entry_t){
		.type = ENTRY_TYPE_SINGLE,
		.content.single_instruction = {
			.mnemonic = "xor",
			.opcode_bytes = {0x35},
			.opcode_length = ONE_B_OPCODE,
			.modrm_required = false,
			.operand_count = COUNT_TWO,
			.operands = {
				{
				.encoding_type = ENCODING_FIXED_REG,
				.operand_width = WIDTH_32,
				.operand_value = { .reg = EAX },
				.operand_role = OPERAND_SRC_DEST,
				.opsize_sensitive = true,
				.addrsize_sensitive = false	
				},
				{
				.encoding_type = ENCODING_IMMEDIATE, // Fixed from encoding_byte
				.operand_width = WIDTH_32,
				.operand_value = {.imm_val = {.value = 0,.extension = EXTENSION_SIGN}},
				.operand_role = OPERAND_SOURCE,
				.opsize_sensitive = true,
				.addrsize_sensitive = false
				}
			}
		}
	};

	primary_table[0x83] = (opcode_table_entry_t){
		.type = ENTRY_TYPE_EXTENSION_GROUP,
        .content.extension_bucket = {
             [OPCODE_EXTENSION_6] = {
                 .mnemonic = "xor",
                 .opcode_bytes = { 0x83 },
                 .opcode_length = ONE_B_OPCODE,
                 .modrm_required = true,
                 .operand_count = COUNT_TWO,
                 .operands = {
                    {
                        .encoding_type = ENCODING_RM,
                        .operand_width = WIDTH_32,
                        .operand_role = OPERAND_SRC_DEST,
                        .opsize_sensitive = true,
                        .addrsize_sensitive = true
                    },
                    {
                        .encoding_type = ENCODING_IMMEDIATE,
                        .operand_width = WIDTH_8,
                        .operand_value = { .imm_val = { .value = 0, .extension = EXTENSION_SIGN } },
                        .operand_role = OPERAND_SOURCE,
                        .opsize_sensitive = false,
                        .addrsize_sensitive = false
                    }
                }
            }
        }
    };

	primary_table[0x80] = (opcode_table_entry_t){
        .type = ENTRY_TYPE_EXTENSION_GROUP,
        .content.extension_bucket = {
            [OPCODE_EXTENSION_6] = { 
                .mnemonic = "xor",
                .opcode_bytes = { 0x80 },
                .opcode_length = ONE_B_OPCODE,
                .modrm_required = true,
                .operand_count = COUNT_TWO,
                .operands = {
                     {
                        .encoding_type = ENCODING_RM,
                        .operand_width = WIDTH_8,
                        .operand_role = OPERAND_SRC_DEST,
                        .opsize_sensitive = false, 
                        .addrsize_sensitive = true  
                     },
                     {
                        .encoding_type = ENCODING_IMMEDIATE,
                        .operand_width = WIDTH_8,
                        .operand_value = { .imm_val = { .value = 0, .extension = EXTENSION_SIGN } },
                        .operand_role = OPERAND_SOURCE,
                        .opsize_sensitive = false,
                        .addrsize_sensitive = false
                     }
                }
             }
         }
     };

	primary_table[0x81] = (opcode_table_entry_t){
        .type = ENTRY_TYPE_EXTENSION_GROUP,
        .content.extension_bucket = {
            [OPCODE_EXTENSION_6] = { 
                .mnemonic = "xor",
                .opcode_bytes = { 0x81 },
                .opcode_length = ONE_B_OPCODE,
                .modrm_required = true,
                .operand_count = COUNT_TWO,
                .operands = {
                    {
                        .encoding_type = ENCODING_RM,
                        .operand_width = WIDTH_32,
                        .operand_role = OPERAND_SRC_DEST,
                        .opsize_sensitive = true,  
                        .addrsize_sensitive = true  
                    },
                    {
                        .encoding_type = ENCODING_IMMEDIATE,
                        .operand_width = WIDTH_32,
                        .operand_value = { .imm_val = { .value = 0, .extension = EXTENSION_SIGN } },
                        .operand_role = OPERAND_SOURCE,
                        .opsize_sensitive = true,  
                        .addrsize_sensitive = false
                    }
                }
            }
        }
    };

	primary_table[0x8F] = (opcode_table_entry_t){
        .type = ENTRY_TYPE_EXTENSION_GROUP,
        .content.extension_bucket = {
            [OPCODE_EXTENSION_0] = { 
                .mnemonic = "pop",
                .opcode_bytes = { 0x8F },
                .opcode_length = ONE_B_OPCODE,
                .modrm_required = true,
                .operand_count = COUNT_ONE,
                .operands = {
                    {
                        .encoding_type = ENCODING_RM,
                        .operand_width = WIDTH_16,
			.operand_role = OPERAND_DESTINATION,
                        .opsize_sensitive = true, 
                        .addrsize_sensitive = true
                    }
                }
            }
        }
    };

	primary_table[0x58] = (opcode_table_entry_t){
        .type = ENTRY_TYPE_SINGLE,
        .content.single_instruction = {
            .mnemonic = "pop",
            .opcode_bytes = { 0x58 },
            .opcode_length = ONE_B_OPCODE,
            .modrm_required = false,
            .operand_count = COUNT_ONE,
            .operands = {
                {
                    .encoding_type = ENCODING_FIXED_REG,
                    .operand_width = WIDTH_32, // Or WIDTH_16 based on 0x66 prefix
                    .operand_value = { .reg = EAX },
                    .operand_role = OPERAND_DESTINATION,
                    .opsize_sensitive = true,
                    .addrsize_sensitive = false
                }
            }
        }
    };

	primary_table[0x1F] = (opcode_table_entry_t){
        .type = ENTRY_TYPE_SINGLE,
        .content.single_instruction = {
            .mnemonic = "pop",
            .opcode_bytes = { 0x1F },
            .opcode_length = ONE_B_OPCODE,
            .modrm_required = false,
            .operand_count = COUNT_ONE,
            .operands = {
                {
                    .encoding_type = ENCODING_FIXED_REG,
                    .operand_width = WIDTH_16,
                    .operand_value = { .reg = DS },
                    .operand_role = OPERAND_DESTINATION,
                    .opsize_sensitive = false,
                    .addrsize_sensitive = false
                }
            }
        }
    };

	primary_table[0x07] = (opcode_table_entry_t){
        .type = ENTRY_TYPE_SINGLE,
        .content.single_instruction = {
            .mnemonic = "pop",
            .opcode_bytes = { 0x07 },
            .opcode_length = ONE_B_OPCODE,
            .modrm_required = false,
            .operand_count = COUNT_ONE,
            .operands = {
                {
                    .encoding_type = ENCODING_FIXED_REG,
                    .operand_width = WIDTH_16, // Segment registers are 16-bit
                    .operand_value = { .reg = ES },
                    .operand_role = OPERAND_DESTINATION,
                    .opsize_sensitive = false,
                    .addrsize_sensitive = false
                }
            }
        }
    };

	primary_table[0x17] = (opcode_table_entry_t){
        .type = ENTRY_TYPE_SINGLE,
        .content.single_instruction = {
            .mnemonic = "pop",
            .opcode_bytes = { 0x17 },
            .opcode_length = ONE_B_OPCODE,
            .modrm_required = false,
            .operand_count = COUNT_ONE,
            .operands = {
                {
                    .encoding_type = ENCODING_FIXED_REG,
                    .operand_width = WIDTH_16, // Segment registers are always 16-bit
                    .operand_value = { .reg = SS },
                    .operand_role = OPERAND_DESTINATION,
                    .opsize_sensitive = false,
                    .addrsize_sensitive = false
                }
            }
        }
    };

	secondary_table[0xA1] = (opcode_table_entry_t){
        .type = ENTRY_TYPE_SINGLE,
        .content.single_instruction = {
            .mnemonic = "pop",
            .opcode_bytes = { 0x0F, 0xA1 },
            .opcode_length = TWO_B_OPCODE,
            .modrm_required = false,
            .operand_count = COUNT_ONE,
            .operands = {
                {
                    .encoding_type = ENCODING_FIXED_REG,
                    .operand_width = WIDTH_16, // Segment registers are always 16-bit
                    .operand_value = { .reg = FS },
                    .operand_role = OPERAND_DESTINATION,
                    .opsize_sensitive = true,
                    .addrsize_sensitive = false
                }
            }
        }
    };

	secondary_table[0xA9] = (opcode_table_entry_t){
        .type = ENTRY_TYPE_SINGLE,
        .content.single_instruction = {
            .mnemonic = "pop",
            .opcode_bytes = { 0x0F, 0xA9 },
            .opcode_length = TWO_B_OPCODE,
            .modrm_required = false,
            .operand_count = COUNT_ONE,
            .operands = {
                {
                    .encoding_type = ENCODING_FIXED_REG,
                    .operand_width = WIDTH_32,   
                    .operand_value = { .reg = GS },
                    .operand_role = OPERAND_DESTINATION,
                    .opsize_sensitive = true,
                    .addrsize_sensitive = false
                }
            }
        }
    };
}

