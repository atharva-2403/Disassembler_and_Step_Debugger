#ifndef REFERENCE_INSTRUCTION_FRAMEWORK_H
#define REFERENCE_INSTRUCTION_FRAMEWORK_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_PREFIXES 4
#define MAX_OPERANDS 3
#define MAX_OPCODE_BYTES 3

typedef enum{

	SOURCE_NONE,
	SOURCE_MODRM_REG,
	SOURCE_MODRM_RM,
	SOURCE_OPCODE_REG,
	SOURCE_IMMEDIATE,
	SOURCE_FIXED_REG,
	SOURCE_IMPLICIT

}operand_source_t;

typedef enum {

       	ENCODING_NONE,
	ENCODING_RM,  //This is the modrm rm
	ENCODING_REG, //This is the modr
	ENCODING_OPCODE_REG,
	ENCODING_IMMEDIATE,
	ENCODING_RELATIVE, 
	ENCODING_FAR_POINTER,
	ENCODING_FIXED_REG, 
	ENCODING_IMPLICIT

} operand_encoding_t;

typedef enum{

	WIDTH_NONE,
	WIDTH_8,
	WIDTH_16,
	WIDTH_32

}operand_width_t;

typedef enum{

	REG_NONE,
	
	AL, BL, CL, DL,
	AH, BH, CH, DH,

	AX, BX, CX, DX,
	SP, BP, SI, DI, 

	EAX, ECX, EDX, EBX, 
	ESP, EBP, ESI, EDI

}register_t;

typedef enum{

	EXTENSION_NONE,
	EXTENSION_ZERO,
	EXTENSION_SIGN

}extension_t;
typedef struct{

	uint32_t value;
	extension_t extension;

}immediate_t;

typedef struct{

	register_t base;
	register_t index;
	uint8_t scale;
	int32_t displacement; 

}memory_t;

typedef struct{

    uint16_t segment_selector;
    uint32_t offset;

}far_pointer_t;
typedef union{

	register_t reg;
	immediate_t imm_val;
	memory_t mem_val;
	int32_t rel_val;
	far_pointer_t far_ptr_val;
	uint32_t const_val;

}operand_value_t;

typedef enum{

	OPCODE_EXTENSION_R = -1, //this is for /r
	OPCODE_EXTENSION_0 = 0,
	OPCODE_EXTENSION_1 = 1,
	OPCODE_EXTENSION_2 = 2, 
	OPCODE_EXTENSION_3 = 3,
	OPCODE_EXTENSION_4 = 4, 
	OPCODE_EXTENSION_5 = 5,
	OPCODE_EXTENSION_6 = 6,
	OPCODE_EXTENSION_7 = 7

}opcode_extension_t;

typedef enum{

	OPERAND_SOURCE,
	OPERAND_DESTINATION,
	OPERAND_SRC_DEST

}operand_role_t;

#ifndef OPCODE_SIZE_T_DEF
#define OPCODE_SIZE_T_DEF
typedef enum{

	ONE_B_OPCODE = 1,
	TWO_B_OPCODE = 2,
	THREE_B_OPCODE = 3

}opcode_size_t;
#endif

typedef enum{
	
	COUNT_ZERO = 0,
	COUNT_ONE = 1,
	COUNT_TWO = 2,
	COUNT_THREE = 3

}operand_count_t;

typedef struct{
	operand_encoding_t encoding_type;
	operand_width_t operand_width;
	operand_value_t operand_value;
	operand_role_t operand_role;

}operand_reference_t;

typedef struct{
	
	uint8_t opcode_bytes[MAX_OPCODE_BYTES];
	uint8_t prefix_count;	
	uint8_t prefixes_needed[MAX_PREFIXES];	
	const char *mnemonic_name;
	opcode_size_t opcode_size;
	opcode_extension_t opcode_extension;
	bool modrm_needed;
	operand_count_t operand_count;
	operand_reference_t operands[MAX_OPERANDS];

}instruction_reference_t;

#endif

