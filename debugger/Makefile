# NOTE :
# sir i used ai for this makefile cuz  the debugger i made works on the handcoded breakpoints and the breakpoints generated
# in your machine may differ from mine so i automated the process of generating the breakpoints
# so these are still hardcoded but you can change them in the Makefile . 

CC      = gcc
CFLAGS  = -Wall -g
LDLIBS  = -lcapstone
ADD_ADDR = $(shell nm test | awk '$$3 == "add" {print "0x"$$1}')

.PHONY: all run clean check-deps

all: ptrace test

check-deps:
	@echo '#include <capstone/capstone.h>' | $(CC) $(CFLAGS) -E -x c - >/dev/null 2>&1 || { echo "capstone not found. Install it with: sudo apt install libcapstone-dev"; exit 1; }

test: test.c
	$(CC) -no-pie -g -O0 test.c -o test

ptrace: ptrace.c test check-deps
	$(CC) $(CFLAGS) -DADD_ADDR=$(ADD_ADDR) ptrace.c -o ptrace $(LDLIBS)

run: all
	./ptrace ./test

clean:
	rm -f ptrace test


