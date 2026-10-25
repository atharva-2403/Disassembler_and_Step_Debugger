#ifndef REFERENCE_INSTRUCTION_FRAMEWORK_H
#define REFERENCE_INSTRUCTION_FRAMEWORK_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_PREFIXES 4
#define MAX_OPERANDS 3
#define MAX_OPCODE_BYTES 3

typedef enum {

       	ENCODING_NONE,
	ENCODING_RM,  		//This is the modrm rm
	ENCODING_REG, 		//This is the modrm reg
	ENCODING_OPCODE_REG,	//possibly redundant.
	ENCODING_IMMEDIATE,	//for immediate values
	ENCODING_RELATIVE, 	//idek what this is 
	ENCODING_FAR_POINTER,	//what is this???
	ENCODING_FIXED_REG, 	//LIKE pop ds
	ENCODING_IMPLICIT	//This is understood 

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
	
	CS, DS, SS, ES, 
	FS, GS,
	
	EAX, ECX, EDX, EBX, 
	ESP, EBP, ESI, EDI

}x86_reg_t;

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

	x86_reg_t base;
	x86_reg_t index;
	uint8_t scale;
	int32_t displacement; 

}memory_t;

typedef struct{

    uint16_t segment_selector;
    uint32_t offset;

}far_pointer_t;

typedef union{

	x86_reg_t reg;
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

typedef enum{

	ONE_B_OPCODE = 1,
	TWO_B_OPCODE = 2,
	THREE_B_OPCODE = 3

}opcode_size_t;

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
	bool opsize_sensitive;
	bool addrsize_sensitive;

}operand_reference_t;

typedef struct{

	const char *mnemonic;	
	uint8_t opcode_bytes[MAX_OPCODE_BYTES];
	opcode_size_t opcode_length;	
	bool modrm_required;
	operand_count_t operand_count;
	operand_reference_t operands[MAX_OPERANDS];

}instruction_reference_t;

#endif

