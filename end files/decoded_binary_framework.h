#ifndef DECODED_BINARY_FRAMEWORK_H
#define DECODED_BINARY_FRAMEWORK_H

#define MAX_OPCODE_LIMIT 3
#define MAX_PREFIX_LIMIT 4
#define MAX_IMMEDT_LIMIT 4
#define MAX_INSTRUCTION_LENGTH 15
#include <stdint.h>

typedef enum{
	YES,
	NO
}validity_t;

typedef struct{
	validity_t is_valid;
	uint8_t prefix_arr[MAX_PREFIX_LIMIT];
	uint8_t no_of_prefix;
}prefix_t;

#ifndef OPCODE_SIZE_T_DEF
#define OPCODE_SIZE_T_DEF
typedef enum{
	ONE_B_OPCODE = 1,
	TWO_B_OPCODE = 2,
	THREE_B_OPCODE = 3
}opcode_size_t;
#endif

typedef struct{
	validity_t is_valid;
	uint8_t bytes[MAX_OPCODE_LIMIT];
	opcode_size_t size;
}opcode_t;

typedef struct{
	validity_t is_valid;
	uint8_t mod;
	uint8_t reg;
	uint8_t rm;
}modrm_t;

typedef struct{
	validity_t is_valid;
	uint8_t scale;
	uint8_t index;
	uint8_t base;
}sib_t;

typedef enum{
	IMM_NONE,
	IMM_8,
	IMM_16,
	IMM_32,
}imm_size_t;

typedef enum{
	DISP0,
	DISP8,
	DISP32
}disp_size_t;

typedef struct{
	validity_t is_valid;
	disp_size_t size;
	uint8_t bytes[4];
}disp_t;

typedef struct{
	validity_t is_valid;
	uint8_t bytes[MAX_IMMEDT_LIMIT];
	imm_size_t size;	
}imm_t;

typedef struct{
	uint8_t raw_bytes[MAX_INSTRUCTION_LENGTH];
	uint8_t length;
	prefix_t prefixes;
	opcode_t opcode;
	modrm_t modrm;
	sib_t sib;
	disp_t disp;
	imm_t immediate;
}decoded_binary;

#endif
