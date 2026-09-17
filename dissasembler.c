#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <executable>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);

    if (fd < 0)
    {
        perror("Could not open file");
        return 1;
    }

    Elf32_Ehdr header;

    ssize_t bytes_read = read(fd, &header, sizeof(header));

    if (bytes_read != sizeof(header))
    {
        printf("Could not read ELF header.\n");
        close(fd);
        return 1;
    }

    // Check ELF magic number 
    if (header.e_ident[EI_MAG0] != ELFMAG0 ||
        header.e_ident[EI_MAG1] != ELFMAG1 ||
        header.e_ident[EI_MAG2] != ELFMAG2 ||
        header.e_ident[EI_MAG3] != ELFMAG3)
    {
        printf("Not a valid ELF file.\n");
        close(fd);
        return 1;
    }

    // Check whether it is 32-bit 
    if (header.e_ident[EI_CLASS] != ELFCLASS32)
    {
        printf("This is not a 32-bit ELF file.\n");
        close(fd);
        return 1;
    }

    // Check whether it is x86 
    if (header.e_machine != EM_386)
    {
        printf("This is not an x86 (32-bit) executable.\n");
        close(fd);
        return 1;
    }

    printf("Valid x86 32-bit ELF file\n\n");

    printf("Entry point : 0x%x\n", header.e_entry);
    printf("Section table offset : 0x%x\n", header.e_shoff);
    printf("Number of sections : %d\n", header.e_shnum);
    printf("Section header size : %d bytes\n", header.e_shentsize);

    close(fd);


   return 0; }
