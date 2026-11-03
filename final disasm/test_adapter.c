#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "adapter.h"

int main(void) {
    /* Sample instructions stream:
     * 55          push %ebp
     * 89 e5       mov  %esp, %ebp
     * 83 ec 10    sub  $0x10, %esp
     * 8d 45 08    lea  0x8(%ebp), %eax
     * 74 08       je   0x8048013
     * c3          ret
     */
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

    /* 1: push %ebp */
    assert(strcmp(res->instructions[0].mnemonic, "push") == 0);
    assert(strcmp(res->instructions[0].cmd, "PUSH") == 0);
    assert(res->instructions[0].start_address == 0x08048000);
    assert(res->instructions[0].length == 1);
    assert(res->instructions[0].dest_reg == REG_EBP);
    assert(res->instructions[0].type == INST_NORMAL);

    /* 2: mov %esp, %ebp */
    assert(strcmp(res->instructions[1].mnemonic, "mov") == 0);
    assert(res->instructions[1].n_operands == 2);
    assert(res->instructions[1].operands[0].reg == EBP);
    assert(res->instructions[1].operands[1].reg == ESP);

    /* 3: sub $0x10, %esp */
    assert(strcmp(res->instructions[2].mnemonic, "sub") == 0);
    assert(res->instructions[2].has_immediate == true);
    assert(res->instructions[2].immediate_val == 0x10);

    /* 4: lea 0x8(%ebp), %eax */
    assert(strcmp(res->instructions[3].mnemonic, "lea") == 0);
    assert(res->instructions[3].operands[1].type == OP_MEMORY);
    assert(res->instructions[3].is_memory_src == true);
    assert(res->instructions[3].mem_src.base == REG_EBP);
    assert(res->instructions[3].mem_src.disp == 8);

    /* 5: je rel8 */
    assert(strcmp(res->instructions[4].mnemonic, "je") == 0);
    assert(res->instructions[4].type == INST_COND_BRANCH);
    assert(res->instructions[4].condition == COND_E);
    assert(res->instructions[4].target_address == 0x08048013);

    /* 6: ret */
    assert(strcmp(res->instructions[5].mnemonic, "ret") == 0);
    assert(res->instructions[5].type == INST_RET);
    assert(res->instructions[5].n_operands == 0);

    /* NULL / safety checks */
    assert(disasm_elf_file(NULL) == NULL);
    assert(disasm_elf_file("non_existent_file.elf") == NULL);
    assert(disasm_for_worklet2(NULL) == NULL);
    assert(disasm_for_worklet2("non_existent_file.elf") == NULL);

    free_adapter_result(res);
    printf("All adapter tests passed.\n");
    return 0;
}
