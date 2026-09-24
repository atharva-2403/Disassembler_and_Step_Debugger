CC = gcc 
CFLAGS = -g

all : 32_bit_elf validator text_extractor information_decoding run

validator : file_validation.c
	$(CC) $(CFLAGS) file_validation.c -o file_validation

text_extractor : extract_text_section.c text_section.h
	$(CC) $(CFLAGS) extract_text_section.c -o extract_text_section

decoder : information_decoding.c text_section.h
	$(CC) $(CFLAGS) information_decoding.c -o information_decoding
clean: 
	rm -rf file_validation extract_text_section

update_git:
	rm -rf /home/phosgene/github_things/Disassembler_and_Step_Debugger/*
	cp /home/phosgene/OS_Project/laboratory-2/* /home/phosgene/github_things/Disassembler_and_Step_Debugger

run: 
	./file_validation 32_bit_elf





