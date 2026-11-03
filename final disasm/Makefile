CC      ?= gcc
CFLAGS  ?= -Wall -Wextra -std=c11 -g -I.

# Core disassembler engine sources
CORE_SRCS = prefix.c decoder.c instruction_table.c semantic_decoder.c
CORE_OBJS = $(CORE_SRCS:.c=.o)

# Full disassembler target
TARGET    = disasm
FULL_SRCS = $(CORE_SRCS) elf32.c formatter.c main.c
FULL_OBJS = $(FULL_SRCS:.c=.o)

.PHONY: all unit_tests test_adapter_bridge test_state clean

# Default: build full disassembler
all: $(TARGET)

$(TARGET): $(FULL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Generic object compilation
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# -----------------------------------------------------------------------------
# Test Suites
# -----------------------------------------------------------------------------

# Unit tests: MC/DC coverage verification suite (97 tests)
unit_tests: $(CORE_OBJS) test_mcdc.o
	$(CC) $(CFLAGS) -o run_unit_tests $^
	@./run_unit_tests
	@rm -f run_unit_tests

# Adapter bridge test: verifies unified API for frontend and debugger
test_adapter_bridge: $(CORE_OBJS) elf32.o formatter.o adapter.o test_adapter.o
	$(CC) $(CFLAGS) -o run_test_adapter $^
	@./run_test_adapter
	@rm -f run_test_adapter

# Decoder state verification suite
test_state: $(CORE_OBJS) test_state.o
	$(CC) $(CFLAGS) -o run_test_state $^
	@./run_test_state
	@rm -f run_test_state

# -----------------------------------------------------------------------------
# Cleanup
# -----------------------------------------------------------------------------

clean:
	rm -f *.o $(TARGET) run_* fib32 qsort32
	@echo "Cleaned up all build artifacts."
