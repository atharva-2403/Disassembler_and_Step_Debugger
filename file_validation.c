#include <stdio.h>
#include <elf.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define VALIDATION_SUCCESS 1
#define VALIDATION_FAILURE 0
#define NEXT_STAGE_FAILURE 2
#define NEXT_STAGE_SUCCESS 3

#define STAGE_NO 1
#define NEXT_STAGE_SOURCE "extract_text_section"

int begin_file_validation(const char *filename){
	printf("STAGE 1 >> Opening File : %s.\n",filename);
	int fd = open(filename,O_RDONLY);
	if(fd == -1){
		perror("STAGE 1 >> Open File Failed ");
		return VALIDATION_FAILURE;
	}
	Elf32_Ehdr elf_header_container;

	printf("STAGE 1 >> Reading the Elf Header from file.\n");
	ssize_t bytes_read = read(fd,&elf_header_container,sizeof(elf_header_container));
	if(bytes_read == -1){
		perror("Elf Header read failed");
		return VALIDATION_FAILURE;
	}

	printf("STAGE 1 >> Verifying File Format : ");
	if(elf_header_container.e_ident[EI_MAG0] == ELFMAG0 && 
	   elf_header_container.e_ident[EI_MAG1] == ELFMAG1 &&
  	   elf_header_container.e_ident[EI_MAG2] == ELFMAG2 &&
	   elf_header_container.e_ident[EI_MAG3] == ELFMAG3){
		printf("ELF Format Detected.\n");
	}
	else{
		printf("Invalid Format Detected, Aborting.\n");
		return VALIDATION_FAILURE;
	}

	printf("STAGE 1 >> Verifying Target Bitness Type : ");
	if(elf_header_container.e_ident[EI_CLASS] == ELFCLASS32){
		printf("32-bit format Architecture detected.\n");
	}
	else{
		printf("Invalid Architecture Format Detected. Aborting.\n");
		return VALIDATION_FAILURE;
	}
	
	printf("STAGE 1 >> Verifying Target Machine Architecture Type : ");
	if(elf_header_container.e_machine == EM_386){
		printf("Intel 32-bit ISA format Detected.\n");
	}
	else{
		printf("Invalid Type Detected. Aborting\n");
		return VALIDATION_FAILURE;
	}
	printf("STAGE 1 >> Completed.\n");
	close(fd);
	return VALIDATION_SUCCESS;
}
int execute_next_stage(char *filename){
	printf("STAGE 1 >> Executing STAGE 2.\n");
	pid_t pid_no = fork();
	if(pid_no == -1){
		return NEXT_STAGE_FAILURE;
	}
	if(pid_no == 0){	
		char *args[] = {"./"NEXT_STAGE_SOURCE,filename,NULL};
		execv("./"NEXT_STAGE_SOURCE,args);
		perror("STAGE 1 >> execv failed\n");
		return NEXT_STAGE_FAILURE;
	}
	return NEXT_STAGE_SUCCESS;
}

	
int main(int argc, char *argv[]){
	if(argc > 2){
		printf("Too many arguments.\n Usage : ./run <elf_file_path>");
		return 1;
	}
	if(argc < 2){
		printf("No enough arguments.\n Usage : ./run <elf_file_path");
		return 1;
	}
	char *filename = argv[1];


	int status = begin_file_validation(filename);
	if(status == VALIDATION_FAILURE) return 0;
	int stage_status = execute_next_stage(filename);
	if(stage_status == NEXT_STAGE_FAILURE){
		perror("STAGE 1 >> Failed to Execute Extraction Stage\n");
		return 1;
	}
	return 0;
}//end of program

