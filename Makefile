CC = gcc 
CFLAGS = -g

all : 32_bit_elf validator text_extractor run

validator : file_validation.c
	$(CC) $(CFLAGS) file_validation.c -o file_validation

text_extractor : extract_text_section.c
	$(CC) $(CFLAGS) extract_text_section.c -o extract_text_section

clean: 
	rm -rf file_validation extract_text_section

run: 
	./file_validation 32_bit_elf





