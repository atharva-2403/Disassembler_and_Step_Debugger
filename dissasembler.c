#include <elf.h> // provided the elf structure and constants
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// 1.1 of Project - ELF VALIDATION.
//Target- 1 : Take an ELF file as an arguement, validate it.
          // then check if its an ELF FILE USING MAGIC NUMBERS && check if its 32 bits

int main(int argc,char *argv[]){
	if(argc == 1){
		printf("no arguments specified\n");
		printf("usage:checkelf <filename>\n");
		return 1;
	}
	if(argc > 2){
		printf("too many arguments.\n");
		printf("usage:checkelf <filename>\n");
		return 1;
	}
	printf("file processing has begun\n");
    
	//the file that needs to be decoded is argv[1]
	
	int file_desc = open(argv[1],O_RDONLY);
	
	if(file_desc < 0){
		perror("cant open file:");
		return 1;
	}
	//opened the file. 
	//now we have to create a container to store the header data.

	//declaring the container.


	//reading the header files.
	Elf32_Ehdr elf_header_container; //Elf32_Ehdr is a structure defined in .elf lib that shows 32bit header.

	//copies bytes opened from file into this variable.
	ssize_t bytes_read = read(file_desc, &elf_header_container, sizeof(elf_header_container));

if (bytes_read != sizeof(elf_header_container)) {
    printf("cant read the complete ELF header.\n");
    close(file_desc);
    return 1;
}

printf("verifying the type of file (checking magic numbers)\n");
	
	if(elf_header_container.e_ident[EI_MAG0] == ELFMAG0 &&
	   elf_header_container.e_ident[EI_MAG1] == ELFMAG1 &&
   	   elf_header_container.e_ident[EI_MAG2] == ELFMAG2 &&
	   elf_header_container.e_ident[EI_MAG3] == ELFMAG3){
		printf("verfication successful.\n 32-bit ELF format Detected.\n");
	}
	else {
    printf("file is not in the ELF format. please input an elf file.\n");
    close(file_desc);
    return 1;
}	
		
	if(elf_header_container.e_ident[EI_CLASS] != ELFCLASS32){    //EI_CLASS == ELFCLASS32 = the ELF file declares the 32 bit class
    printf("Not a 32-bit ELF File Format\n");
    close(file_desc);
    return 1;
}
	
if (elf_header_container.e_machine != EM_386) {    //e_machine == EM_38 the ELF file declares the Intel 386 architecture family (32-bit x86)
    printf("Not an x86 (32-bit) ELF file.\n");
    close(file_desc);
    return 1;
}

	off_t section_header_offset = elf_header_container.e_shoff; //where does section table header point? = e_shoff
	off_t section_header_index_number = elf_header_container.e_shstrndx;


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

if (lseek(file_desc, section_header_offset, SEEK_SET) == -1) {
    perror("could not find section header"); 						//moving pointer to section header. 
    close(file_desc);
    return 1;
}

//e_shroff -> 0x1365 -> lseek -> 0x1365 -> section header 

// Array to hold section headers
if (elf_header_container.e_shnum == 0 ||
    elf_header_container.e_shentsize != sizeof(Elf32_Shdr)) {
    printf("Invalid section header information.\n");
    close(file_desc);
    return 1;
}

Elf32_Shdr *section_headers =
    malloc(elf_header_container.e_shnum * sizeof(Elf32_Shdr));

if (section_headers == NULL) {
    perror("cant allocate section headers");
    close(file_desc);
    return 1;
}

size_t section_headers_size =
    elf_header_container.e_shnum * sizeof(Elf32_Shdr);

bytes_read = read(file_desc, section_headers, section_headers_size);

if (bytes_read != (ssize_t)section_headers_size) {
    printf("cant read section headers completely.\n");
    free(section_headers);
    close(file_desc);
    return 1;
}
// Milestone 2 
// so far, we  head to section table heading by using eshorff(offset) , then lseek to locate the section headers and store in array.
// find section name string table -> find .text -> get .text offset,address -> read .text bytes

//section name table Concept : Section headers contain secton name table which contain offset that points to .text. 

	if (section_header_index_number >= elf_header_container.e_shnum) {
    printf("Invalid section name table index.\n");
    free(section_headers);
    close(file_desc);
    return 1;
}
Elf32_Shdr section_name_table = section_headers[section_header_index_number];

	if (section_name_table.sh_type != SHT_STRTAB) {
    printf("Invalid section name string table.\n");
    free(section_headers);
    close(file_desc);
    return 1;
}

// Find the section name string table in the file.
off_t string_table_offset = section_name_table.sh_offset;

if (lseek(file_desc, string_table_offset, SEEK_SET) == -1) {
    perror("Cannot find section name string table");
    free(section_headers);
    close(file_desc);
    return 1;
}

// Size of string table, array with section names, read it.
if (section_name_table.sh_size == 0) {
    printf("Section name string table is empty.\n");
    free(section_headers);
    close(file_desc);
    return 1;
}

char *section_names = malloc(section_name_table.sh_size);

if (section_names == NULL) {
    perror("Could not allocate section names");
    free(section_headers);
    close(file_desc);
    return 1;
}

bytes_read = read(file_desc, section_names, section_name_table.sh_size);

if (bytes_read != (ssize_t)section_name_table.sh_size) {
    printf("Could not read section names completely.\n");
    free(section_names);
    free(section_headers);
    close(file_desc);
    return 1;
}

//Milestone 3 : Get .text offset
//so far we have section header and section names array
//sh_name is an offse to the actual name

Elf32_Shdr *text_section = NULL;

for (size_t i = 0; i < elf_header_container.e_shnum; i++) {

    if (section_headers[i].sh_name >= section_name_table.sh_size) {
        continue;
    }

    char *section_name =
        section_names + section_headers[i].sh_name;

    size_t remaining_size =
        section_name_table.sh_size - section_headers[i].sh_name;

    if (memchr(section_name, '\0', remaining_size) == NULL) {
        continue;
    }

    if (strcmp(section_name, ".text") == 0) {
        text_section = &section_headers[i];
        break;
    }
}

if (text_section == NULL) {
    printf(".text section not found\n");
    free(section_names);
    free(section_headers);
    close(file_desc);
    return 1;
}
printf(".text section found!\n");

//text_section is pointing towards .text 
printf("Offset: %u\n", text_section->sh_offset);
printf("Address: 0x%x\n", text_section->sh_addr);
printf("Size: %u bytes\n", text_section->sh_size);

//Milestone 3: read .text
//go to .text

if (lseek(file_desc, text_section->sh_offset, SEEK_SET) == -1) {
    perror("Could not find .text section");
    free(section_names);
    free(section_headers);
    close(file_desc);
    return 1;
}

// Space for bytes of .text
if (text_section->sh_size == 0) {
    printf(".text section is empty.\n");
    free(section_names);
    free(section_headers);
    close(file_desc);
    return 1;
}

unsigned char *text_bytes = malloc(text_section->sh_size);

if (text_bytes == NULL) {
    perror("Could not allocate .text bytes");
    free(section_names);
    free(section_headers);
    close(file_desc);
    return 1;
}

bytes_read = read(file_desc, text_bytes, text_section->sh_size);

if (bytes_read != (ssize_t)text_section->sh_size) {
    printf("Could not read .text bytes completely.\n");
    free(text_bytes);
    free(section_names);
    free(section_headers);
    close(file_desc);
    return 1;
}

// Print extracted bytes to verify them.
printf("\n.text bytes:\n");

for (size_t i = 0; i < text_section->sh_size; i++) {
    printf("%02X ", text_bytes[i]);
}

printf("\n");

// Clean up allocated memory and close the file.
free(text_bytes);
free(section_names);
free(section_headers);
close(file_desc);

return 0;
}
