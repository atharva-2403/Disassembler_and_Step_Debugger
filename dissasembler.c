#include <elf.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// 1.1 of Project - ELF VALIDATION.
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


	off_t section_header_offset = elf_header_container.e_shoff; //where does section table header point? = e_shoff
	off_t section_header_index_number = elf_header_container.e_shstrndx;
		
	}


	//------- START OF 1.2 SEARCHING .TEXT SECTION AND EXTRACTING ITS BYTES.----------

	//1.2 includes 
	//a) find section table header b) find .text c) extract .text bytes 

	//high level understanding 
	// 1. section header table
	   
	//2.section name string satble
	   
	// 3.find .text
	   
	// 4.get its offset + address + size
	   
	// 5.read .text bytes


	//------

	// Milestone 1 
	//find the section headers, and store it in an array.

	if(lseek(fd, section_header_offset, SEEK_SET) == -1){       //this is moving file pointer to section header
	printf("coulnd find section header");
	return 1; }

	//e_shroff -> 0x1365 -> lseek -> 0x1365 -> section header 

	Elf32_Shdr section_headers[elf_header_container.e_shnum]; 	//array to hold section headers

	read(fd, section_headers, sizeof(section_headers)); 

	// Milestone 2 
	// so far, we  head to section table heading by using eshorff(offset) , then lseek to locate the section headers and store in array.
// find section name string table -> find .text -> get .text offset,address -> read .text bytes

//section name table Concept : Section headers contain secton name table which contain offset that points to .text. 

Elf32_Shdr section_name_table = section_headers[section_header_index_number];

if (lseek(fd, string_table_offset, SEEK_SET) == -1) {
    printf("cant find section name string table\n");
    return 1;
}

// size of string table , array with section names , read it.
section_name_table.sh_size
char section_names[section_name_table.sh_size];
read(fd, section_names, sizeof(section_names));

//Milestone 3 : Get .text offset
//so far we have section header and section names array
//sh_name is an offse to the actual name

Elf32_Shdr *text_section = NULL;

for(int i=0; i < elf_header_container.e_shnum; i++) {
	char *section_name =section_names + section_headers[i].sh_name;
	
	if (strcmp(section_name, ".text") == 0) {
        text_section = &section_headers[i];
        break;
    }
}

if (text_section == NULL) {
    printf(".text section not found\n");
    return 1;
}

printf(".text section found!\n");

//text_section is pointing towards .text 
printf("Offset: %u\n", text_section->sh_offset);
printf("Address: 0x%x\n", text_section->sh_addr);
printf("Size: %u bytes\n", text_section->sh_size);

//Milestone 3: read .text
//go to .text

if (lseek(fd, text_section->sh_offset, SEEK_SET) == -1) {
    printf("Could not find .text section\n");
    return 1;
}

//space for bytes
unsigned char text_bytes[text_section->sh_size];
read(fd, text_bytes, text_section->sh_size);
