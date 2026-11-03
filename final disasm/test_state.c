#include <stdio.h>
#include <stdint.h>
#include "decoder.h"
#include "semantic_decoder.h"

static void run_test(const char *name, uint8_t *bytes, int len, uint32_t addr) {
    decoded_binary bin;
    int slen = decode_instruction(bytes, 0, len, &bin);
    if (slen <= 0) {
        printf("[FAIL] %s: physical slicing failed\n", name);
        return;
    }
    instruction_reference_t inst;
    if (decode_semantic(&bin, addr, &inst) != 0) {
        printf("[FAIL] %s: semantic decoding failed\n", name);
        return;
    }
    printf("[PASS] %-25s -> Mnemonic: %-6s (Length: %d bytes, Operands: %d)\n",
           name, inst.mnemonic_name, bin.length, (int)inst.operand_count);
}

int main(void) {
    printf("\n=== Current Decoder State Verification Suite ===\n\n");

    uint8_t t1[] = {0x55};
    run_test("PUSH %ebp", t1, sizeof(t1), 0x08048000);

    uint8_t t2[] = {0x89, 0xE5};
    run_test("MOV %esp, %ebp", t2, sizeof(t2), 0x08048001);

    uint8_t t3[] = {0x83, 0xEC, 0x10};
    run_test("SUB $0x10, %esp", t3, sizeof(t3), 0x08048003);

    uint8_t t4[] = {0x8D, 0x45, 0x08};
    run_test("LEA 0x8(%ebp), %eax", t4, sizeof(t4), 0x08048006);

    uint8_t t5[] = {0xF7, 0xF9};
    run_test("IDIV %ecx", t5, sizeof(t5), 0x08048009);

    uint8_t t6[] = {0x0F, 0x84, 0x20, 0x00, 0x00, 0x00};
    run_test("JE rel32", t6, sizeof(t6), 0x0804800B);

    uint8_t t7[] = {0xC3};
    run_test("RET", t7, sizeof(t7), 0x08048011);

    uint8_t t8[] = {0x66, 0xB8, 0x34, 0x12};
    run_test("MOV $0x1234, %ax (0x66)", t8, sizeof(t8), 0x08048012);

    uint8_t t9[] = {0xF0, 0x83, 0x00, 0x01};
    run_test("LOCK ADD $0x1, (%eax)", t9, sizeof(t9), 0x08048016);

    uint8_t t10[] = {0x65, 0x8B, 0x10};
    run_test("MOV %gs:(%eax), %edx", t10, sizeof(t10), 0x0804801A);

    uint8_t t11[] = {0x67, 0x8B, 0x00};
    run_test("MOV (%bx,%si), %eax (0x67)", t11, sizeof(t11), 0x0804801D);

    printf("\nAll test cases successfully decoded!\n\n");
    return 0;
}
