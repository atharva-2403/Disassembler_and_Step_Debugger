#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "text_section.h"

#define DECODING_SUCCESS 2
#define DECODING FAILURE 3
#define PIPE_READ_DESCRIPTOR 5

int begin_decoding(char *file_name){
	printf("STAGE 3 >> DECODING PHASE BEGINS\n");
	printf("STAGE 3 >> Creating struct to receive information.\n");
	text_section_info info;
	printf("STAGE 3 >> Piping information from STAGE 2\n");
	
	int fd = read(PIPE_READ_DESCRIPTOR,&info,sizeof(info));
	if(fd == -1){
		perror("Information Piping failure");
		return DECODING_FAILURE;
	}
	printf("STAGE 3 >> Closing Pipe.\n");
	printf("STAGE 3 >> Opening the ELF.\n");

	int fd_elf = open(
	off_t status = lseek(

	return DECODING_SUCCESS;
}
int main(){
	begin_decoding();
}
//end of program.
