#include <elf.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
char FILE_NAME[512];
void clear_buffer(){
	printf("\n");
}
void initiate_next_stage(){
	char path[32] = "./extract_text_section";
        char *args[] = {"./extract_text_section",FILE_NAME,NULL};
	pid_t pid_number;
	pid_number = fork();
	if(pid_number == 0){
		execv(path,args);
	}
}
int main(int argc,char *argv[]){
	if(argc == 1){
		printf("No arguments specified.\n");
		printf("Usage : checkelf <filename>\n");
		return 1;
	}
	if(argc > 2){
		printf("Too many arguments.\n");
		printf("Usage : checkelf <filename>\n");
		return 1;
	}

	strcpy(FILE_NAME,argv[1]);
	printf("Begin File Processing\n");
	int file_desc = open(FILE_NAME,O_RDONLY);
	if(file_desc < 0){
		perror("Could Not open file:");
		return 1;
	}
	Elf32_Ehdr elf_header_container;
	ssize_t bytes_read = read(file_desc,&elf_header_container,sizeof(elf_header_container));
	printf("Verifying the type of file......\n");
	
	if(elf_header_container.e_ident[EI_MAG0] == ELFMAG0 &&
	   elf_header_container.e_ident[EI_MAG1] == ELFMAG1 &&
   	   elf_header_container.e_ident[EI_MAG2] == ELFMAG2 &&
	   elf_header_container.e_ident[EI_MAG3] == ELFMAG3){
		printf("Verfication Success.\n 32-bit ELF format Detected.\n");
	}
	else{
		printf("File is not in the ELF Format. Please input an elf file");
		return 1;
	}		
	if(elf_header_container.e_ident[EI_CLASS] != ELFCLASS32){
		printf("Not a 32-bit ELF File Format\n");
		return 1;
	}

	initiate_next_stage();
}
//end of program 


