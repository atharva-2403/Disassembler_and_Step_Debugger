#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

/*
void begin_extraction(char *filename){
	
	int fd = open(filename,O_RDONLY);
	Elf32_Ehdr elf_header_container;
	
	size_t bytes_read = read(fd,&elf_header_container,sizeof(Elf32_Ehdr));
	
	off_t section_header_offset = elf_header_container.shoff;
	if(lseek(fd,section_header_offset,SEEK_SET) == (off_t - 1)){
		printf("Location Not found\n");
		close(fd);
		return;
	}
	else{
		printf("Section header Address Located and Verified");
	}
	
	Elf32_Shdr section_header_container;
	size_t bytes_read = read(fd,&section_header_container,sizeof(section_header_container));
		
	//First we will find out the address of the shstrtab_hdr_offset 
	
	off_t shstrtab_offset = elf_header_container.e_shoff + ( elf_header_container.e_shstrndx * elf_header_container.e_shentsize )

	if(lseek(fd,shstrtab_offset,SEEK_SET) == (off_t)-1){
		perror("Lseek to shstrtab header failed");
		close(fd);
		return;
	}
	
	//if(read(fd,&shstrtab_offset,sizeof(shstrtab_hdr))
} */
char FILEX[32];
int main(int argc,char *argv[]){
	
	strcpy(FILEX,argv[1]);
	
	printf("Performing Text Extraction on File : %s",FILEX);
	printf("\n");
	
	return 0;



}
