#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "adapter.h"

int main(void) {
    printf("\n=== Testing Unified adapter.h / adapter.c ===\n\n");

    // Machine code stream:
    // 55                push %ebp
    // 89 e5             mov %esp, %ebp
    // 83 ec 10          sub $0x10, %esp
    // 8d 45 08          lea 0x8(%ebp), %eax
    // 74 08             je  0x8048013 (addr 0x8048009 + 2 + 8 = 0x8048013)
    // c3                ret
    unsigned char test_bytes[] = {
        0x55,
        0x89, 0xE5,
        0x83, 0xEC, 0x10,
        0x8D, 0x45, 0x08,
        0x74, 0x08,
        0xC3
    };

    DisasmResult *res = disasm_to_adapter(test_bytes, sizeof(test_bytes), 0x08048000);
    assert(res != NULL);
    assert(res->count == 6);

    printf("Total instructions decoded into DisasmResult: %u\n\n", res->count);

    // -------------------------------------------------------------
    // 1. PUSH %ebp
    // -------------------------------------------------------------
    printf("[1] Addr: 0x%x, Len: %d, Cmd: %s, Mnemonic: %s, Operands: %u, Text: '%s'\n",
           res->instructions[0].addr, res->instructions[0].len,
           res->instructions[0].cmd, res->instructions[0].mnemonic,
           res->instructions[0].n_operands, res->instructions[0].text);
    // Backward compatibility
    assert(strcmp(res->instructions[0].cmd, "PUSH") == 0);
    assert(res->instructions[0].operands[0].type == OP_REGISTER);
    assert(res->instructions[0].operands[0].reg == EBP);
    // instruction.h.txt requirements
    assert(strcmp(res->instructions[0].mnemonic, "push") == 0);
    assert(res->instructions[0].start_address == 0x08048000);
    assert(res->instructions[0].length == 1);
    assert(res->instructions[0].raw_decoded.length == 1);
    assert(res->instructions[0].raw_decoded.raw_bytes[0] == 0x55);
    assert(res->instructions[0].dest_reg == REG_EBP);
    assert(res->instructions[0].type == INST_NORMAL);

    // -------------------------------------------------------------
    // 2. MOV %esp, %ebp
    // -------------------------------------------------------------
    printf("[2] Addr: 0x%x, Len: %d, Cmd: %s, Mnemonic: %s, Operands: %u, Text: '%s'\n",
           res->instructions[1].addr, res->instructions[1].len,
           res->instructions[1].cmd, res->instructions[1].mnemonic,
           res->instructions[1].n_operands, res->instructions[1].text);
    assert(strcmp(res->instructions[1].cmd, "MOV") == 0);
    assert(strcmp(res->instructions[1].mnemonic, "mov") == 0);
    assert(res->instructions[1].n_operands == 2);
    assert(res->instructions[1].operands[0].reg == EBP);
    assert(res->instructions[1].operands[1].reg == ESP);
    assert(res->instructions[1].dest_reg == REG_EBP);
    assert(res->instructions[1].src_reg == REG_ESP);

    // -------------------------------------------------------------
    // 3. SUB $0x10, %esp
    // -------------------------------------------------------------
    printf("[3] Addr: 0x%x, Len: %d, Cmd: %s, Mnemonic: %s, Operands: %u, Text: '%s'\n",
           res->instructions[2].addr, res->instructions[2].len,
           res->instructions[2].cmd, res->instructions[2].mnemonic,
           res->instructions[2].n_operands, res->instructions[2].text);
    assert(strcmp(res->instructions[2].cmd, "SUB") == 0);
    assert(strcmp(res->instructions[2].mnemonic, "sub") == 0);
    assert(res->instructions[2].has_immediate == true);
    assert(res->instructions[2].immediate_val == 0x10);

    // -------------------------------------------------------------
    // 4. LEA 0x8(%ebp), %eax
    // -------------------------------------------------------------
    printf("[4] Addr: 0x%x, Len: %d, Cmd: %s, Mnemonic: %s, Operands: %u, Text: '%s'\n",
           res->instructions[3].addr, res->instructions[3].len,
           res->instructions[3].cmd, res->instructions[3].mnemonic,
           res->instructions[3].n_operands, res->instructions[3].text);
    assert(strcmp(res->instructions[3].cmd, "LEA") == 0);
    assert(strcmp(res->instructions[3].mnemonic, "lea") == 0);
    // Backward compatibility memory assertions
    assert(res->instructions[3].operands[1].type == OP_MEMORY);
    assert(res->instructions[3].operands[1].mem.base == EBP);
    assert(res->instructions[3].operands[1].mem.offset == 8);
    // Unified memory assertions
    assert(res->instructions[3].operands[1].mem.disp == 8);
    assert(res->instructions[3].operands[1].mem.scale == 1);
    assert(res->instructions[3].is_memory_src == true);
    assert(res->instructions[3].mem_src.base == REG_EBP);
    assert(res->instructions[3].mem_src.disp == 8);
    printf("    -> Memory Operand: base = EBP, disp = %d, scale = %u\n",
           res->instructions[3].operands[1].mem.disp,
           res->instructions[3].operands[1].mem.scale);

    // -------------------------------------------------------------
    // 5. JE rel8 (Branch testing for Debugger)
    // -------------------------------------------------------------
    printf("[5] Addr: 0x%x, Len: %d, Cmd: %s, Mnemonic: %s, Type: %d, Cond: %d, Target: 0x%x, Text: '%s'\n",
           res->instructions[4].addr, res->instructions[4].len,
           res->instructions[4].cmd, res->instructions[4].mnemonic,
           res->instructions[4].type, res->instructions[4].condition,
           res->instructions[4].target_address, res->instructions[4].text);
    assert(strcmp(res->instructions[4].mnemonic, "je") == 0);
    assert(res->instructions[4].type == INST_COND_BRANCH);
    assert(res->instructions[4].condition == COND_E);
    assert(res->instructions[4].has_target_address == true);
    assert(res->instructions[4].target_address == 0x08048013);

    // -------------------------------------------------------------
    // 6. RET
    // -------------------------------------------------------------
    printf("[6] Addr: 0x%x, Len: %d, Cmd: %s, Mnemonic: %s, Type: %d, Text: '%s'\n",
           res->instructions[5].addr, res->instructions[5].len,
           res->instructions[5].cmd, res->instructions[5].mnemonic,
           res->instructions[5].type, res->instructions[5].text);
    assert(strcmp(res->instructions[5].cmd, "RET") == 0);
    assert(strcmp(res->instructions[5].mnemonic, "ret") == 0);
    assert(res->instructions[5].type == INST_RET);
    assert(res->instructions[5].n_operands == 0);

    free_adapter_result(res);
    printf("\n[SUCCESS] All unified adapter assertions passed cleanly!\n\n");
    return 0;
}
