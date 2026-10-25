#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "text_section.h"
#include "prefix_metadata.h"
#include "decoded_instruction_standard.h"
#include "reference_table_standard.h"

#define DECODING_SUCCESS 2
#define DECODING_FAILURE 3
#define PIPE_READ_DESCRIPTOR 5


//creating the decoder function now
const PrefixInfo* return_prefix(uint8_t byte){
	if(prefix_table[byte].is_prefix) return &prefix_table[byte];
	else return NULL;
}

const int scanPrefixStream(uint8_t *begin,uint8_t *end, decoded_instruction_definition *instr){
	
	*instr = (decoded_instruction_definition){0};
	const uint8_t *current = begin;
	instr->prefix_count = 0;

	while(current!=end){
		uint8_t byte = *current;
		const PrefixInfo *prefix = return_prefix(byte);
		if(prefix == NULL) break;
		if(instr->prefix_count < 4){
			instr->prefixes[instr->prefix_count] = *prefix;
			switch(byte){
				case 0xF0:
					instr->lock_enabled = true;
					break;
				case 0xF2:
					instr->repnz_enabled = true;
					break;
				case 0xF3:
					instr->rep_enabled = true;
					break;
				case 0x26:
				case 0x2E:
				case 0x36:
				case 0x3E:
				case 0x64:
				case 0x65:
					instr->seg_override_enabled = true;
					instr->segment_reg = (segment_t)byte;
					break;
				case 0x66:
					instr->opsize_override_enabled = true;
					break;
				case 0x67:
					instr->addrsize_override_enabled = true;
					break;
			}
			instr->prefix_count++;
		}
		current++;
	}
	return instr->prefix_count;
}

void decoder(uint8_t *buffer,int size){
	
	//calculating the end of the text section
	uint8_t *end = buffer + size;
	//creating an instruction array;
	decoded_instruction_definition instructions[1024] = {0};
	int instruction_count = 0;
	bool instruction_over = false;
	uint8_t *current = buffer;
	
	while(current!=end){

		if(instruction_over){
			instruction_count++;
			instruction_over = false;
		}
		int no_of_prefixes =  scanPrefixStream(current,end,&instructions[instruction_count]);
		current = current + no_of_prefixes;

	}
	printf("\nDECODING COMPLETE\n");

	printf("Instructions Parsed = %d\n",instruction_count);
	
	for(int i=0;i<instruction_count;i++){
		int pref_count = instructions[i].prefix_count;
		for(int j=0;j<pref_count;j++){
			printf("%s ",instructions[i].prefixes[j].name);
		}
		printf("\n");
	}
	printf("POINT APLHA\n");




}











int begin_decoding(char *filename){
	printf("\nSTAGE 3 >> DECODING PHASE BEGINS\n");
	printf("STAGE 3 >> Creating struct to receive information.\n");
	text_section_info info;
	printf("STAGE 3 >> Piping information from STAGE 2\n");
	
	ssize_t bytes_rd = read(PIPE_READ_DESCRIPTOR,&info,sizeof(info));
	if(bytes_rd == -1){
		perror("Information Piping failure");
		return DECODING_FAILURE;
	}
	printf("STAGE 3 >> Closing Pipe.\n");
	close(PIPE_READ_DESCRIPTOR);

	printf("STAGE 3 >> Opening the ELF.\n");	
	int fd_elf = open(filename,O_RDONLY);
	if(fd_elf == -1){
		perror("STAGE 3 >> Failure to open ELF.");
		close(fd_elf);
		return DECODING_FAILURE;
	}
	off_t status = lseek(fd_elf,info.text_section_offset,SEEK_SET);
	if(status == (off_t)-1){
		perror("STAGE 3 >> Lseek failed");
		close(fd_elf);
		return DECODING_FAILURE;
	}
	printf("STAGE 3 >> Creating a storage buffer\n");
	unsigned char *text_section_buffer = malloc(info.size);
	
	printf("STAGE 3 >> Buffer created, Beginning Read.\n");

	ssize_t bytes_read = read(fd_elf,text_section_buffer,info.size);
	if(bytes_read == -1){
		perror("STAGE 3 >> Failure to read ELF");
		close(fd_elf);
		return DECODING_FAILURE;
	}
	printf("STAGE 3 >> Read Complete, Closing file.\n");
	close(fd_elf);
	printf("STAGE 3 >> File Closed Successfully\n");
	//declaring the count;
	printf("info.size = %d\n",info.size);
	for(int i=0;i<info.size;i++){
		printf("%02x ",text_section_buffer[i]);
	}
	printf("\n");
	
	printf("STAGE 3 >> Invoking the decoder.\n");
	//decoder(text_section_buffer,info.size);
	printf("ALERT : Loading instruction definitions\n");
	printf("ALERT : Initialising the opcode tables\n");
	init_opcode_tables();
	printf("ALERT : Opcode tables initialised\n");
	printf("ALERT : Reading the primary opcode table:\n");

	for (int i = 0; i < 256; i++) {
    	opcode_table_entry_t *ptr = get_opcode_entry(TABLE_PRIMARY, i);

   	 if (ptr == NULL || ptr->type == ENTRY_TYPE_NONE) {
        	continue; // Skip unmapped / empty slots entirely!
    	}

   	 if (ptr->type == ENTRY_TYPE_SINGLE) {
        	printf("0x%02X -> INSTRUCTION FOUND : %s\n", i, ptr->content.single_instruction.mnemonic);
    	}
   	else if (ptr->type == ENTRY_TYPE_EXTENSION_GROUP) {
       	for (int j = 0; j < 8; j++) {
        	    if (ptr->content.extension_bucket[j].mnemonic != NULL) {
               	 printf("0x%02X /%d -> EXTENSION INSTRUCTION FOUND : %s\n",i, j, ptr->content.extension_bucket[j].mnemonic);
            }
        }
    }
}	

	
	return DECODING_SUCCESS;
}
int main(int argc, char *argv[]){
	
	if(argc>2 || argc<2) return 1;
	char *filename = argv[1];
	begin_decoding(filename);
	
}
//end of program.
