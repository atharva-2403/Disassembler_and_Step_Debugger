#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(int argc, char *argv[]) {
  int fd;
  Elf64_Ehdr *ehdr;
  fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  char *buf;
  off_t size;
  size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  buf = malloc(size);
  read(fd, buf, size);
  close(fd);
  ehdr = (Elf64_Ehdr *)buf;
  printf("ELF Header\n");
  printf("e_ident :");
  for (int i = 0; i < EI_NIDENT; i++) {
    printf("%lx ", (unsigned long)ehdr->e_ident[i]);
  }
  printf("\n");
  printf("e_type : %lx\n", (unsigned long)ehdr->e_type);
  printf("e_machine : %lx\n", (unsigned long)ehdr->e_machine);
  printf("e_version : %lx\n", (unsigned long)ehdr->e_version);
  printf("e_entry : %lx\n", (unsigned long)ehdr->e_entry);
  printf("e_phoff : %lx\n", (unsigned long)ehdr->e_phoff);
  printf("e_shoff : %lx\n", (unsigned long)ehdr->e_shoff);
  printf("e_flags : %lx\n", (unsigned long)ehdr->e_flags);
  printf("e_ehsize : %lx\n", (unsigned long)ehdr->e_ehsize);
  printf("e_phentsize : %lx\n", (unsigned long)ehdr->e_phentsize);
  printf("e_phnum : %lx\n", (unsigned long)ehdr->e_phnum);
  printf("e_shentsize : %lx\n", (unsigned long)ehdr->e_shentsize);
  printf("e_shnum : %lx\n", (unsigned long)ehdr->e_shnum);
  printf("e_shstrndx : %lx\n", (unsigned long)ehdr->e_shstrndx);

  Elf64_Phdr *phdr;
  phdr = (Elf64_Phdr *)(buf + ehdr->e_phoff);
  printf("\n\n\n");

  for (int i = 0; i < ehdr->e_phnum; i++) {
    printf("PROGRAM HEADER %d\n", i);
    printf("p_type : %lx\n", (unsigned long)phdr[i].p_type);
    printf("p_offset : %lx\n", (unsigned long)phdr[i].p_offset);
    printf("p_vaddr : %lx\n", (unsigned long)phdr[i].p_vaddr);
    printf("p_paddr : %lx\n", (unsigned long)phdr[i].p_paddr);
    printf("p_filesz : %lx\n", (unsigned long)phdr[i].p_filesz);
    printf("p_memsz : %lx\n", (unsigned long)phdr[i].p_memsz);
    printf("p_flags : %lx\n", (unsigned long)phdr[i].p_flags);
    printf("p_align : %lx\n", (unsigned long)phdr[i].p_align);
    printf("\n\n\n\n");
  }

  Elf64_Shdr *shdr;
  shdr = (Elf64_Shdr *)(buf + ehdr->e_shoff);
  Elf64_Shdr *shstrtab = &shdr[ehdr->e_shstrndx];
  char *shstrtab_buf;
  shstrtab_buf = (char *)(buf + shstrtab->sh_offset);

  for (int i = 0; i < ehdr->e_shnum; i++) {
    printf("SECTION HEADER %d\n", i);
    printf("sh_name : %s\n", shstrtab_buf + shdr[i].sh_name);
    printf("sh_type : %lx\n", (unsigned long)shdr[i].sh_type);
    printf("sh_flags : %lx\n", (unsigned long)shdr[i].sh_flags);
    printf("sh_addr : %lx\n", (unsigned long)shdr[i].sh_addr);
    printf("sh_offset : %lx\n", (unsigned long)shdr[i].sh_offset);
    printf("sh_size : %lx\n", (unsigned long)shdr[i].sh_size);
    printf("sh_link : %lx\n", (unsigned long)shdr[i].sh_link);
    printf("sh_info : %lx\n", (unsigned long)shdr[i].sh_info);
    printf("sh_addralign : %lx\n", (unsigned long)shdr[i].sh_addralign);
    printf("sh_entsize : %lx\n", (unsigned long)shdr[i].sh_entsize);
    printf("\n\n\n\n");

    // check if section is .text
    if (strcmp(shstrtab_buf + shdr[i].sh_name, ".text") == 0) {
      printf("section is .text\n");
      Elf64_Shdr *text_shdr = &shdr[i];
      Elf64_Shdr *text_shstrtab = &shdr[ehdr->e_shstrndx];
      char *text_shstrtab_buf;
      text_shstrtab_buf = (char *)(buf + text_shstrtab->sh_offset);
      for (int j = 0; j < text_shdr->sh_size; j++) {
        printf("%x ", text_shstrtab_buf[j]);
      }
      printf("\n");
    }
    printf("\n\n\n\n");
  }

  return 0;
}
