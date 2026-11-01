CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c11 -g -I.

STATE_SRCS = prefix.c decoder.c instruction_table.c semantic_decoder.c
STATE_OBJS = $(STATE_SRCS:.c=.o)

FULL_TARGET = disasm
FULL_EXTRA_SRCS = elf32.c formatter.c main.c
FULL_SRCS = $(STATE_SRCS) $(wildcard $(FULL_EXTRA_SRCS))

.PHONY: all state clean test_state

# default -> full disassembler
all: $(FULL_TARGET)

$(FULL_TARGET): $(STATE_OBJS)
	@if [ -f "main.c" ] && [ -f "formatter.c" ]; then \
		$(CC) $(CFLAGS) -o $@ $(FULL_SRCS); \
		echo "--> Full disassembler '$(FULL_TARGET)' built successfully!"; \
	else \
		echo "================================================================================"; \
		echo " [NOTE] 'make' is reserved for building the entire disassembler (final target)."; \
		echo " The remaining parts (formatter.c & main.c) are scheduled for the next phase."; \
		echo ""; \
		echo " To compile everything implemented up to the current state, run:"; \
		echo "     make state"; \
		echo ""; \
		echo " To run the verification suite on the current decoder state, run:"; \
		echo "     make test_state"; \
		echo "================================================================================"; \
	fi

# compile state modules
state: $(STATE_OBJS)
	@echo "================================================================================"
	@echo " [SUCCESS] All modules for the current decoder state compiled cleanly!"
	@echo " Object files generated:"
	@echo "   - prefix.o             (Intel prefix classifier)"
	@echo "   - decoder.o            (Stage 1: Byte extraction & physical slicing)"
	@echo "   - instruction_table.o  (IA-32 instruction knowledge base)"
	@echo "   - semantic_decoder.o   (Stage 2: Operands, memory, and register resolution)"
	@echo "================================================================================"

# .c -> .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# test decoder state
test_state: $(STATE_OBJS) test_state.o
	$(CC) $(CFLAGS) -o run_test_state test_state.o $(STATE_OBJS)
	@./run_test_state
	@rm -f run_test_state

# test passed format
test_passed: $(STATE_OBJS) passed_format.o test_passed_format.o
	$(CC) $(CFLAGS) -o run_test_passed test_passed_format.o passed_format.o $(STATE_OBJS)
	@./run_test_passed
	@rm -f run_test_passed

# demos
demo_fib: $(FULL_TARGET)
	@gcc -m32 -O0 -fno-pie -no-pie -o fib32 test_fib.c
	@echo "=== Recursive Fibonacci Execution ==="
	@./fib32
	@echo ""
	@echo "=== Disassembly of <fibonacci> via ./disasm ==="
	@./disasm fib32 | grep -A 28 "8049176:"

demo_qsort: $(FULL_TARGET)
	@gcc -m32 -O0 -fno-pie -no-pie -o qsort32 test_quicksort.c
	@echo "=== Quicksort Execution ==="
	@./qsort32
	@echo ""
	@echo "=== Disassembly of <quicksort> via ./disasm ==="
	@./disasm qsort32 | grep -A 27 "8049265:"

demo: demo_fib demo_qsort

clean:
	rm -f *.o $(FULL_TARGET) run_test_state run_test_passed fib32 qsort32 .state_test_runner*
	@echo "Cleaned up all build artifacts."
