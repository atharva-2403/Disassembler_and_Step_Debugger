


CC = gcc
CFLAGS = -g -O0

all : elf_file_validation extract_text_section flush run

elf_file_validation : elf_file_validation.c
	$(CC) $(CLANG) elf_file_validation.c -o elf_file_validation

extract_text_section : extract_text_section.c
	$(CC) $(CLANG) extract_text_section.c -o extract_text_section

flush : flush.c
	$(CC) $(CFLAG) flush.c -o flush

run: elf_file_validation extract_text_section flush
	./elf_file_validation 32_bit_elf
	./flush

run_extraction : extract_text_section 32_bit_elf
	./extract_text_section 32_bit_elf

clean : 
	rm -rf elf_file_validation extract_text_section flush
