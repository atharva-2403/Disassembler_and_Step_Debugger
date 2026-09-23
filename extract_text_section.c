#include <stdio.h>
#include <elf.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define EXTRACTION_FAILURE 1
#define EXTRACTION_SUCCES
int extract_text_section(char *filename){
	
	int fd = open(filename,O_RDONLY);
	printf("STAGE 2 >> Opening File : %s\n",filename);
	if(fd == -1){
		perror("Unable to open file");
		return EXTRACTION_FAILURE;
	}

	printf("STAGE 2 >> Reading the ELF header.\n");
	Elf32_Ehdr elf_header_container;
	ssize_t bytes_read = read(fd,&elf_header_container,sizeof(elf_header_container));
	if(bytes_read == -1){
		perror("STAGE 2 >> Failure to read elf header.");
		return EXTRACTION_FAILURE;
	}
	off_t section_header_offset = elf_header_container.e_shoff; 
	printf("STAGE 2 >> Extracting the .shstrtab section.\n");

	off_t shstrtab_offset = section_header_offset + elf_header_container.e_shstrndx * elf_header_container.e_shentsize;
	lseek(fd,shstrtab_offset,SEEK_SET);
	Elf32_Shdr shstrtab_container;
	bytes_read = read(fd,&shstrtab_container,sizeof(shstrtab_container));
	
	char *shstrtab_data = malloc(shstrtab_container.sh_size);
	lseek(fd,shstrtab_container.sh_offset,SEEK_SET);
	read(fd,shstrtab_data,shstrtab_container.sh_size);

	printf("STAGE 2 >> Read shstrtab contents complete.\n");
	printf("STAGE 2 >> Extracting the .text section.\n");
	printf("STAGE 2 >> Begin Search.\n");

	lseek(fd,section_header_offset,SEEK_SET);
	off_t text_section_offset;
	for(int i=0; i < elf_header_container.e_shnum; i++){
		Elf32_Shdr section_header_container;
		ssize_t bytes_read = read(fd,&section_header_container,sizeof(section_header_container));
		if(bytes_read == -1) continue;
		char *name = &shstrtab_data[section_header_container.sh_name];
		if(strcmp(name,".text") == 0){
			text_section_offset = section_header_container.sh_offset;
			printf("STAGE 2 >> .text section located (offset:%u)\n",text_section_offset);
		}
	}
	printf("STAGE 2 >> Closing file : %s\n",filename);
	printf("STAGE 2 >> Completed.\n");
	close(fd);
}
int main(int argc, char *argv[]){
	if(argc > 2) return 1;
	if(argc < 2) return 1;
	char *filename = argv[1]; 
	extract_text_section(filename);
	return 0;
}
