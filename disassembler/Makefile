CC = gcc
CFLAGS = -I frameworks/ -I metadata/

all: 
	./file_validation 32_bit_elf

help:
	# make		: run the program
	# make compile	: compile all the files.
	# make clean	: clear up all the executables, but you need to run "make compile" again to run it

compile: file_validation extract_text_section information_decoding

file_validation: file_validation.c
	$(CC) file_validation.c -o file_validation

extract_text_section: extract_text_section.c
	$(CC) extract_text_section.c -o extract_text_section $(CFLAGS)

information_decoding: information_decoding.c reference_table.c
	$(CC) information_decoding.c reference_table.c -o information_decoding $(CFLAGS)

clean:
	rm -f file_validation extract_text_section information_decoding
