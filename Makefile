
CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGET = disassembler
SOURCES = dissasembler.c decoder.c instruction_table.c
HEADERS = decoder.h instruction_table.h instruction_framework.h
TEST_FILE = test32

.PHONY: all test check clean

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

test: $(TARGET)
	@test -f $(TEST_FILE) || { echo "Error: test32 not found"; exit 1; }
	./$(TARGET) ./$(TEST_FILE)

check: test

clean:
	rm -f $(TARGET)
