#include <stdio.h>
#include <elf.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "text_section.h"

#define EXTRACTION_FAILURE 1
#define EXTRACTION_SUCCESS 2
#define NEXT_STAGE_INITIATION_FAIL 3
#define NEXT_STAGE_INITIATION_SUCCESS 4
#define PIPE_READ_DESCRIPTOR 5
#define NEXT_STAGE "information_decoding"

int execute_next_stage(text_section_info info,char *filename){
        
        int pipefd[2];
        printf("STAGE 2 >> Creating Pipe into STAGE 3\n");
        if(pipe(pipefd) == -1){
                perror("Pipe Failed\n");
                return NEXT_STAGE_INITIATION_FAIL;
        }
        pid_t pid = fork();
        if(pid == -1){
                perror("Fork Failed");
                return NEXT_STAGE_INITIATION_FAIL;
        }
        if(pid>0){
                printf("STAGE 2 >> Piping information into STAGE 3\n");
                close(pipefd[0]);
                write(pipefd[1],&info,sizeof(info));
                close(pipefd[1]);
        }
        else{
                close(pipefd[1]);
                int fd_dup = dup2(pipefd[0],PIPE_READ_DESCRIPTOR);
                if(fd_dup == -1){
                        perror("STAGE 2 >> dup2 failed.");
                        return NEXT_STAGE_INITIATION_FAIL;
                }
                char *argv[] = {"./"NEXT_STAGE, filename, NULL};
                char *source = "./"NEXT_STAGE;
                execv(source,argv);
                perror("STAGE 2 >> Exec Failed.\n");
                return NEXT_STAGE_INITIATION_FAIL;
        }
}
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
	Elf32_Shdr text_section_header;

	for(int i=0; i < elf_header_container.e_shnum; i++){
		Elf32_Shdr section_header_container;
		ssize_t bytes_read = read(fd,&section_header_container,sizeof(section_header_container));
		if(bytes_read == -1) continue;
		char *name = &shstrtab_data[section_header_container.sh_name];
		if(strcmp(name,".text") == 0){
			text_section_offset = section_header_container.sh_offset;
			text_section_header = section_header_container;
			printf("STAGE 2 >> .text section located (offset:%u)\n",text_section_offset);
		}
	}
	
	printf("STAGE 2 >> .text section size: %u bytes\n",text_section_header.sh_size);
	
	printf("STAGE 2 >> Creating struct for information transfer.\n");
	
	text_section_info text_container;
	text_container.text_section_offset = text_section_header.sh_offset;
	text_container.size = text_section_header.sh_size;
	text_container.virtual_address = text_section_header.sh_addr;

	printf("STAGE 2 >> Struct creation complete.\n");
	printf("STAGE 2 >> Closing file : %s\n",filename);
	close(fd);
	printf("STAGE 2 >> Completed.\n");
	printf("STAGE 2 >> Initiating STAGE 3\n");
	execute_next_stage(text_container,filename);
	return EXTRACTION_SUCCESS;
}
int main(int argc, char *argv[]){
	if(argc > 2) return 1;
	if(argc < 2) return 1;
	char *filename = argv[1]; 
	extract_text_section(filename);
	fflush(stdout);
	return 0;
}
