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



	primary_table[0x5E] = (opcode_table_entry_t){
		.type = ENTRY_TYPE_SINGLE,
		.content.single_instruction = {
			.mnemonic = "pop",
			.opcode_bytes = { 0x5E },
			.opcode_length = ONE_B_OPCODE,
			.modrm_required = false,
			.operand_count = COUNT_ONE,
			.operands = {
				{
					.encoding_type = ENCODING_OPCODE_REG,
					.operand_width = WIDTH_32,
					.operand_value = { .reg = ESI },
					.operand_role = OPERAND_DESTINATION,
					.opsize_sensitive = true,
					.addrsize_sensitive = false
				}
			}
		}
	};

	primary_table[0x89] = (opcode_table_entry_t){
		.type = ENTRY_TYPE_SINGLE,
		.content.single_instruction = {
			.mnemonic = "mov",
			.opcode_bytes = { 0x89 },
			.opcode_length = ONE_B_OPCODE,
			.modrm_required = true,
			.operand_count = COUNT_TWO,
			.operands = {
				{
					.encoding_type = ENCODING_RM,
					.operand_width = WIDTH_32,
					.operand_role = OPERAND_DESTINATION,
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

	primary_table[0x83] = (opcode_table_entry_t){
		.type = ENTRY_TYPE_EXTENSION_GROUP,
		.content.extension_bucket = {
			// [0] through [3] can be added later as needed (add, or, adc, sbb)
			[4] = {
				.mnemonic = "and",
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
						.operand_width = WIDTH_SIGN_EXT_8,
						.operand_role = OPERAND_SOURCE,
						.opsize_sensitive = false,
						.addrsize_sensitive = false
					}
				}
			},
			// [5] = sub, [6] = xor, [7] = cmp can be filled here too as you hit them
		}
	};

	primary_table[0x50] = (opcode_table_entry_t){
		.type = ENTRY_TYPE_SINGLE,
		.content.single_instruction = {
			.mnemonic = "push",
			.opcode_bytes = { 0x50 },
			.opcode_length = ONE_B_OPCODE,
			.modrm_required = false,
			.operand_count = COUNT_ONE,
			.operands = {
				{
					.encoding_type = ENCODING_OPCODE_REG,
					.operand_width = WIDTH_32,
					.operand_value = { .reg = EAX }, // Base/default register; overridden dynamically by resolveModrmReg via opcode lower 3 bits
					.operand_role = OPERAND_SOURCE,
					.opsize_sensitive = true,
					.addrsize_sensitive = false
				}
			}
		}
	};
	
	primary_table[0x52] = (opcode_table_entry_t){
		.type = ENTRY_TYPE_SINGLE,
		.content.single_instruction = {
			.mnemonic = "push",
			.opcode_bytes = { 0x52 },
			.opcode_length = ONE_B_OPCODE,
			.modrm_required = false,
			.operand_count = COUNT_ONE,
			.operands = {
				{
					.encoding_type = ENCODING_OPCODE_REG,
					.operand_width = WIDTH_32,
					.operand_value = { .reg = EDX },
					.operand_role = OPERAND_SOURCE,
					.opsize_sensitive = true,
					.addrsize_sensitive = false
				}
			}
		}
	};

	primary_table[0x54] = (opcode_table_entry_t){
		.type = ENTRY_TYPE_SINGLE,
		.content.single_instruction = {
			.mnemonic = "push",
			.opcode_bytes = { 0x54 },
			.opcode_length = ONE_B_OPCODE,
			.modrm_required = false,
			.operand_count = COUNT_ONE,
			.operands = {
				{
					.encoding_type = ENCODING_OPCODE_REG,
					.operand_width = WIDTH_32,
					.operand_value = { .reg = ESP },
					.operand_role = OPERAND_SOURCE,
					.opsize_sensitive = true,
					.addrsize_sensitive = false
				}
			}
		}
	};
}

