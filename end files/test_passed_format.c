#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "passed_format.h"

int main(void) {
    printf("\n=== Testing passed_format.h / passed_format.c ===\n\n");

    // Machine code stream:
    // 55                push %ebp
    // 89 e5             mov %esp, %ebp
    // 83 ec 10          sub $0x10, %esp
    // 8d 45 08          lea 0x8(%ebp), %eax
    // c3                ret
    unsigned char test_bytes[] = {
        0x55,
        0x89, 0xE5,
        0x83, 0xEC, 0x10,
        0x8D, 0x45, 0x08,
        0xC3
    };

    DisasmResult *res = disasm_to_passed_format(test_bytes, sizeof(test_bytes), 0x08048000);
    assert(res != NULL);
    assert(res->count == 5);

    printf("Total instructions decoded into DisasmResult: %u\n\n", res->count);

    // 1. PUSH %ebp
    printf("[1] Addr: 0x%x, Len: %d, Cmd: %s, Operands: %u\n",
           res->instructions[0].addr, res->instructions[0].len,
           res->instructions[0].cmd, res->instructions[0].n_operands);
    assert(strcmp(res->instructions[0].cmd, "PUSH") == 0);
    assert(res->instructions[0].operands[0].type == OP_REGISTER);
    assert(res->instructions[0].operands[0].reg == EBP);

    // 2. MOV %esp, %ebp
    printf("[2] Addr: 0x%x, Len: %d, Cmd: %s, Operands: %u\n",
           res->instructions[1].addr, res->instructions[1].len,
           res->instructions[1].cmd, res->instructions[1].n_operands);
    assert(strcmp(res->instructions[1].cmd, "MOV") == 0);
    assert(res->instructions[1].n_operands == 2);

    // 3. SUB $0x10, %esp
    printf("[3] Addr: 0x%x, Len: %d, Cmd: %s, Operands: %u\n",
           res->instructions[2].addr, res->instructions[2].len,
           res->instructions[2].cmd, res->instructions[2].n_operands);
    assert(strcmp(res->instructions[2].cmd, "SUB") == 0);

    // 4. LEA 0x8(%ebp), %eax
    printf("[4] Addr: 0x%x, Len: %d, Cmd: %s, Operands: %u\n",
           res->instructions[3].addr, res->instructions[3].len,
           res->instructions[3].cmd, res->instructions[3].n_operands);
    assert(strcmp(res->instructions[3].cmd, "LEA") == 0);
    // Operand 1 is memory: base=EBP, offset=8
    assert(res->instructions[3].operands[1].type == OP_MEMORY);
    assert(res->instructions[3].operands[1].mem.base == EBP);
    assert(res->instructions[3].operands[1].mem.offset == 8);
    printf("    -> Memory Operand: base = EBP, offset = %d\n", res->instructions[3].operands[1].mem.offset);

    // 5. RET
    printf("[5] Addr: 0x%x, Len: %d, Cmd: %s, Operands: %u\n",
           res->instructions[4].addr, res->instructions[4].len,
           res->instructions[4].cmd, res->instructions[4].n_operands);
    assert(strcmp(res->instructions[4].cmd, "RET") == 0);
    assert(res->instructions[4].n_operands == 0);

    free_passed_format_result(res);
    printf("\n[SUCCESS] All passed_format assertions passed cleanly!\n\n");
    return 0;
}
