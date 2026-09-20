#include <elf.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//Target- 1 : Take an ELF file as an arguement, validate it.
            // then check if its an ELF FILE USING MAGIC NUMBERS && check if its 64 bits

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
	printf("Begin File Processing\n");
    
	//the file that needs to be decoded is argv[1]
	
	int file_desc = open(argv[1],O_RDONLY);
	if(file_desc < 0){
		perror("Could Not open file:");
		return 1;
	}
	//opened the file. 
	//now we have to create a container to store the header data.

	//declaring the container.
	
	Elf32_Ehdr elf_header_container;

	//Elf32_Ehdr is a struct that comes under the header lib elf.h
	
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


	off_t section_header_offset = elf_header_container.e_shoff;
	off_t section_header_index_number = elf_header_container.e_shstrndx;
	
	off_t the_Actual_thing = section_header_offset + section_header_index_number * sizeof(Elf64_Shdr);

}
//end of program 
