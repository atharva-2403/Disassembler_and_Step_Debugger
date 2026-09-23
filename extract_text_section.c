#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <elf.h>

int main(int argc, char *argv[]){
	char *filename = argv[1];
	printf("Extracting text section from file : %s\n",filename);
	return 0;
}

