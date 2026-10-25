#include <capstone/capstone.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>
#define BP_ADDR 0x40114e  // address of the main this is our first breakpoint
#define ADD_ADDR 0x401136 // address of add

typedef struct {
  unsigned long addr;
  unsigned long orig; // the one saved byte
  int active;
} Bp; // struct foor breakpoints

Bp bps[16];    // a table to upto 16 breakpoints
int nbp = 0;   // how many breakpoints are set
int alive = 1; // is the program alive

struct user_regs_struct regs;
csh handle;

// extracts the instruction bytes from the given address and decodes them using
// capstone disassembler
void show_instructions(csh handle, pid_t pid, unsigned long long rip) {
  unsigned char buf[16]; // in x86 a instruction is max 15 bytes
  for (int i = 0; i < 2; i++) {
    errno = 0;
    long word = ptrace(PTRACE_PEEKDATA, pid, rip + i * 8, 0);
    if (word == -1 && errno) {
      perror("peek");
      return;
    }
    memcpy(buf + i * 8, &word, 8); // copy the bytes to the buffer
  }
  for (int i = 0; i < nbp; i++) {
    if (bps[i].active && bps[i].addr >= rip && bps[i].addr < rip + 16) {
      buf[bps[i].addr - rip] = bps[i].orig & 0xFF;
    }
  }
  cs_insn *insn;
  size_t count = cs_disasm(handle, buf, sizeof(buf), rip, 1, &insn);

  if (count > 0) {
    printf("0x%llx:\t%s\t%s\n", rip, insn[0].mnemonic, insn[0].op_str);
    cs_free(insn, count);
  } else {
    printf("0x%llx:\t\t(cannot decode)\n", rip);
  }
}

// enables a breakpoint by putting the 0xcc byte at the adddress
void bp_enable(Bp *bp, pid_t pid) {
  if (bp->active) {
    return;
  }
  bp->active = 1;
  bp->orig = ptrace(PTRACE_PEEKDATA, pid, bp->addr, 0);
  ptrace(PTRACE_POKEDATA, pid, bp->addr, (bp->orig & ~0xFFL) | 0xCC);
  nbp++;
}

// disables a breakpoint by restoring the original byte that means we put the
// real byte back to its addresss
void bp_disable(Bp *bp, pid_t pid) {
  if (!bp->active) {
    return;
  }
  bp->active = 0;
  ptrace(PTRACE_POKEDATA, pid, bp->addr, bp->orig);
  nbp--;
}

// is there a breakpoint at this address
int bp_find(unsigned long addr) {
  for (int i = 0; i < nbp; i++) {
    if (bps[i].addr == addr) {
      return i;
    }
  }
  return -1;
}

// steps once and checks if there is a breakpoint at the current address and if
// so disables it and enables it again
int step_once(pid_t pid) {
  int status;
  ptrace(PTRACE_GETREGS, pid, 0, &regs);
  int idx = bp_find(regs.rip);
  if (idx != -1) {
    bp_disable(&bps[idx], pid);
  }
  ptrace(PTRACE_SINGLESTEP, pid, 0, 0);
  waitpid(pid, &status, 0);
  if (WIFEXITED(status)) {
    printf("Child exited with status %d\n", WEXITSTATUS(status));
    alive = 0;
    return 0;
  }
  if (WIFSIGNALED(status)) {
    printf("Child killed by signal %d\n", WTERMSIG(status));
    alive = 0;
    return 0;
  }
  if (idx != -1) {
    bp_enable(&bps[idx], pid);
  }
  return 1;
}

// continues the program until it hits a breakpoint or dies
void do_cont(pid_t pid) {
  int status;
  if (!step_once(pid)) {
    return;
  }
  ptrace(PTRACE_CONT, pid, 0, 0);
  waitpid(pid, &status, 0);
  if (WIFEXITED(status)) {
    printf("Child exited with status %d\n", WEXITSTATUS(status));
    alive = 0;
    return;
  }
  if (WIFSIGNALED(status)) {
    printf("Child killed by signal %d\n", WTERMSIG(status));
    alive = 0;
    return;
  }
  if (WSTOPSIG(status) != SIGTRAP) {
    printf("stopped by signal %d\n", WSTOPSIG(status));
    return;
  }
  ptrace(PTRACE_GETREGS, pid, 0, &regs);
  int idx = bp_find(regs.rip - 1);
  if (idx != -1) {
    regs.rip = regs.rip - 1;
    ptrace(PTRACE_SETREGS, pid, 0, &regs);
    printf("breakpoint hit at :    0x%llx\n", regs.rip);
    show_instructions(handle, pid, regs.rip);
  }
}

int main(int argc, char *argv[]) {
  pid_t pid;
  pid = fork();
  if (pid == 0) {
    // we are in child
    ptrace(PTRACE_TRACEME, 0, 0, 0); // ok so  my parent is going to trace me
    execvp(argv[1], argv + 1); // now we will exec our child and make it the
                               // target program that we want to debug
    perror("exec");
    _exit(1);
  } else {
    int status;
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
      printf("failed to start capstone\n");
      return 1;
    }
    waitpid(pid, &status, 0);
    printf("stopped by signal %d\n", WSTOPSIG(status));
    bps[nbp].addr = ADD_ADDR;
    bp_enable(&bps[nbp], pid);

    ptrace(PTRACE_GETREGS, pid, 0, &regs);
    show_instructions(handle, pid, regs.rip);
    while (alive) {
      do_cont(pid);
      if (alive) {
        printf("rdi = %llu, rsi = %llu\n", regs.rdi, regs.rsi);

        // single stepping 5 times after hitting our handcoded breakpoints
        for (int i = 0; i < 5; i++) {
          if (!step_once(pid))
            break;
          ptrace(PTRACE_GETREGS, pid, 0, &regs);
          show_instructions(handle, pid, regs.rip);
        }
      }
    }
    cs_close(&handle);
  }
}
