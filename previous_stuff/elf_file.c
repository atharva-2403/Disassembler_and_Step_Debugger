#include <elf.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//functions to create : 
//
//	1) is_elf_file.
//	2) take input from the main
//	3) printf all the file parameters.


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
		

	//the filename is argv[1]
	
	int file_desc = open(argv[1],O_RDONLY);
	if(file_desc < 0){
		perror("Could Not open file:");
		return 1;
	}
	//opened the file. 
	//now we have to create a container to store the header data.
	//as i have read the manpage, i have understood tht the header container data type should be Elf32_Ehdr.
	
	//declaring the container.
	
	Elf32_Ehdr elf_header_container;

	//Elf32_Ehdr is basically a struct.
	

	//now, as i read the manpage further, i understand tht inside the Elf32_Ehdr struct, there is a field calld : e_ident.
	//
	//e_ident is basically an array that deal with the magic bytes. Magic bytes help you verify if the file is an elf file or not. 
	
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


	//now reading the section header. 
	

	//As i read further, i understand that the text file is stored at a location.
	//This location is specified in the section header area.
	//
	//The location of the section header is given by the e_shoff member of the elf header of the file. 
	//inside the section header area, we will have to navigate to the section header index whose offset is given by the e_shstrndx member of the elf header of the file.
	


	//the .text section header is one of the entries in the section header of the elf file. 
	//Turns out you have to search for the thing by iterating through it. 
	//			    Why do you have to iterate?
	//	Because the section header of the elf file is the array of indexes of all the sections in the file. 
	//
	//				How do you iterate?
	//	You jump over the blocks to reach the beginning of each block. 
	//	How much do you jump over? You jump over size given by sizeof(Elf32_Ehdr) 
	//	then you check if it is the text section.
	//	And then you iterate to the next one. 



	off_t section_header_offset = elf_header_container.e_shoff;
	off_t section_header_index_number = elf_header_container.e_shstrndx;
	
	off_t the_Actual_thing = section_header_offset + section_header_index_number * sizeof(Elf64_Shdr);

	if(lseek,
	return 0;
}
//end of program 


