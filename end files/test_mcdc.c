#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include "decoder.h"
#include "instruction_table.h"
#include "semantic_decoder.h"
#include "prefix.h"

static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define MCDC_CHECK(cond, label) do { \
    g_tests_run++; \
    if (cond) { \
        printf("    [PASS] %s\n", label); \
        g_tests_passed++; \
    } else { \
        printf("    [FAIL] %s (Line %d)\n", label, __LINE__); \
        g_tests_failed++; \
    } \
} while (0)

/* ========================================================================= */
/* FUNCTION 1: needs_sib()                                                   */
/* Logic under test:                                                         */
/*   if (has_prefix_67(d)) return 0;                                         */
/*   if (d->modrm.mod != 3 && d->modrm.rm == 4) return 1;                   */
/*                                                                           */
/* Decision: (mod != 3 && rm == 4) with override from prefix 0x67            */
/* Conditions:                                                               */
/*   C1: mod != 3                                                            */
/*   C2: rm == 4                                                             */
/*   C0: prefix 0x67 override                                                */
/* ========================================================================= */
static void test_mcdc_needs_sib(void) {
    printf("\n--- Test Suite 1: needs_sib() [MC/DC Coverage] ---\n");
    printf("Decision: if (d->modrm.mod != 3 && d->modrm.rm == 4) [with 0x67 override]\n");

    decoded_binary d;

    // Test 1: Baseline True (C1=True [mod=0 != 3], C2=True [rm=4], C0=False) -> SIB needed (1)
    init_decoded(&d);
    d.modrm.is_valid = YES;
    d.modrm.mod = 0; // C1 = True
    d.modrm.rm = 4;  // C2 = True
    MCDC_CHECK(needs_sib(&d) == 1,
               "TC1 [Baseline True]: mod=0, rm=4, no-prefix -> needs_sib = 1");

    // Test 2: Toggle C1 only (C1=False [mod=3], C2=True [rm=4], C0=False) -> SIB not needed (0)
    // Demonstrates C1 independently affects outcome (1 -> 0)
    init_decoded(&d);
    d.modrm.is_valid = YES;
    d.modrm.mod = 3; // C1 = False
    d.modrm.rm = 4;  // C2 = True
    MCDC_CHECK(needs_sib(&d) == 0,
               "TC2 [Toggle C1]: mod=3, rm=4 -> needs_sib = 0 (C1 independence shown)");

    // Test 3: Toggle C2 only (C1=True [mod=0], C2=False [rm=5], C0=False) -> SIB not needed (0)
    // Demonstrates C2 independently affects outcome (1 -> 0)
    init_decoded(&d);
    d.modrm.is_valid = YES;
    d.modrm.mod = 0; // C1 = True
    d.modrm.rm = 5;  // C2 = False
    MCDC_CHECK(needs_sib(&d) == 0,
               "TC3 [Toggle C2]: mod=0, rm=5 -> needs_sib = 0 (C2 independence shown)");

    // Test 4: Override Condition C0 (C1=True, C2=True, C0=True [prefix 0x67 present]) -> 0
    // Demonstrates 0x67 address-size override suppresses SIB in 16-bit addressing
    init_decoded(&d);
    d.modrm.is_valid = YES;
    d.modrm.mod = 0;
    d.modrm.rm = 4;
    d.prefixes.is_valid = YES;
    d.prefixes.no_of_prefix = 1;
    d.prefixes.prefix_arr[0] = 0x67; // C0 = True
    MCDC_CHECK(needs_sib(&d) == 0,
               "TC4 [Toggle C0]: prefix 0x67 present -> needs_sib = 0 (16-bit override shown)");
}

/* ========================================================================= */
/* FUNCTION 2: decode_immediate() [F6/F7 Group 3 Sizing]                     */
/* Logic under test:                                                         */
/*   if ((opcode == 0xF6 || opcode == 0xF7) && reg > 1) { count = 0; }      */
/*                                                                           */
/* Decision: (A || B) && C                                                   */
/* Conditions:                                                               */
/*   A: opcode == 0xF6                                                       */
/*   B: opcode == 0xF7                                                       */
/*   C: reg > 1                                                              */
/* ========================================================================= */
static void test_mcdc_decode_immediate_f6_f7(void) {
    printf("\n--- Test Suite 2: decode_immediate() F6/F7 Group [MC/DC Coverage] ---\n");
    printf("Decision: if ((opcode == 0xF6 || opcode == 0xF7) && reg > 1) -> count = 0\n");

    decoded_binary d;
    uint8_t buffer[8] = {0x11, 0x22, 0x33, 0x44};

    // Test 1: (A=True [0xF6], B=False, C=True [reg=2: NOT]) -> count=0, immediate suppressed
    init_decoded(&d);
    d.opcode.size = ONE_B_OPCODE;
    d.opcode.bytes[0] = 0xF6; // A = True
    d.modrm.is_valid = YES;
    d.modrm.reg = 2;          // C = True (reg > 1)
    decode_immediate(buffer, 0, sizeof(buffer), &d);
    MCDC_CHECK(d.immediate.is_valid != YES,
               "TC1 [A=T, B=F, C=T]: 0xF6 with reg=2 (NOT) -> no immediate (count=0)");

    // Test 2: Toggle A (A=False [0x80], B=False, C=True [reg=2]) -> count > 0 (imm is parsed)
    // Demonstrates condition A independently toggles the outcome (count=0 vs count=1)
    init_decoded(&d);
    d.opcode.size = ONE_B_OPCODE;
    d.opcode.bytes[0] = 0x80; // A = False
    d.modrm.is_valid = YES;
    d.modrm.reg = 2;          // C = True
    decode_immediate(buffer, 0, sizeof(buffer), &d);
    MCDC_CHECK(d.immediate.is_valid == YES && d.immediate.size == IMM_8,
               "TC2 [A=F, B=F, C=T]: 0x80 with reg=2 -> imm parsed (A independence shown)");

    // Test 3: Toggle B (A=False, B=True [0xF7], C=True [reg=2: NOT]) -> count=0
    // Demonstrates condition B independently toggles outcome to True when compared to TC2
    init_decoded(&d);
    d.opcode.size = ONE_B_OPCODE;
    d.opcode.bytes[0] = 0xF7; // B = True
    d.modrm.is_valid = YES;
    d.modrm.reg = 2;          // C = True
    decode_immediate(buffer, 0, sizeof(buffer), &d);
    MCDC_CHECK(d.immediate.is_valid != YES,
               "TC3 [A=F, B=T, C=T]: 0xF7 with reg=2 (NOT) -> no immediate (B independence shown)");

    // Test 4: Toggle C (A=True [0xF6], B=False, C=False [reg=0: TEST]) -> count=1 (TEST has imm)
    // Demonstrates condition C independently toggles outcome to False when compared to TC1
    init_decoded(&d);
    d.opcode.size = ONE_B_OPCODE;
    d.opcode.bytes[0] = 0xF6; // A = True
    d.modrm.is_valid = YES;
    d.modrm.reg = 0;          // C = False (reg <= 1, TEST opcode)
    decode_immediate(buffer, 0, sizeof(buffer), &d);
    MCDC_CHECK(d.immediate.is_valid == YES && d.immediate.size == IMM_8,
               "TC4 [A=T, B=F, C=F]: 0xF6 with reg=0 (TEST) -> imm parsed (C independence shown)");
}

/* ========================================================================= */
/* FUNCTION 3: decode_disp() [ModR/M SIB Disp32 Handling]                    */
/* Logic under test:                                                         */
/*   if (mod == 0 && sib.is_valid == YES && sib.base == 5) { count = 4; }   */
/*                                                                           */
/* Decision: A && B && C                                                     */
/* Conditions:                                                               */
/*   A: mod == 0                                                             */
/*   B: sib.is_valid == YES                                                  */
/*   C: sib.base == 5                                                        */
/* ========================================================================= */
static void test_mcdc_decode_disp(void) {
    printf("\n--- Test Suite 3: decode_disp() SIB Disp32 [MC/DC Coverage] ---\n");
    printf("Decision: if (mod == 0 && sib.is_valid == YES && sib.base == 5) -> disp32 (4 bytes)\n");

    decoded_binary d;
    uint8_t buffer[8] = {0x00, 0x10, 0x00, 0x00}; // 0x00001000

    // Test 1: Baseline True (A=True [mod=0], B=True [is_valid], C=True [base=5]) -> count = 4
    init_decoded(&d);
    d.modrm.is_valid = YES;
    d.modrm.mod = 0;
    d.modrm.rm = 4; // indicates SIB
    d.sib.is_valid = YES;
    d.sib.scale = 0;
    d.sib.index = 0;
    d.sib.base = 5; // [EBP base in mod 0 means disp32]
    decode_disp(buffer, 0, sizeof(buffer), &d);
    MCDC_CHECK(d.disp.is_valid == YES && d.disp.size == DISP32,
               "TC1 [Baseline True]: mod=0, sib.valid=YES, sib.base=5 -> disp32 decoded");

    // Test 2: Toggle A (A=False [mod=1], B=True, C=True) -> count = 1 (disp8, not disp32)
    // Demonstrates A independently changes the displacement sizing
    init_decoded(&d);
    d.modrm.is_valid = YES;
    d.modrm.mod = 1;
    d.modrm.rm = 4;
    d.sib.is_valid = YES;
    d.sib.base = 5;
    decode_disp(buffer, 0, sizeof(buffer), &d);
    MCDC_CHECK(d.disp.is_valid == YES && d.disp.size == DISP8,
               "TC2 [Toggle A]: mod=1, sib.base=5 -> disp8 decoded (A independence shown)");

    // Test 3: Toggle B (A=True [mod=0], B=False [no sib], rm=0) -> count = 0 (no displacement)
    // Demonstrates B independently changes the outcome
    init_decoded(&d);
    d.modrm.is_valid = YES;
    d.modrm.mod = 0;
    d.modrm.rm = 0; // [eax], no SIB
    d.sib.is_valid = NO;
    decode_disp(buffer, 0, sizeof(buffer), &d);
    MCDC_CHECK(d.disp.is_valid != YES,
               "TC3 [Toggle B]: mod=0, sib.valid=NO, rm=0 -> no disp (B independence shown)");

    // Test 4: Toggle C (A=True [mod=0], B=True, C=False [sib.base=0, e.g. [eax]]) -> count = 0
    // Demonstrates C independently changes the outcome
    init_decoded(&d);
    d.modrm.is_valid = YES;
    d.modrm.mod = 0;
    d.modrm.rm = 4;
    d.sib.is_valid = YES;
    d.sib.base = 0; // base = EAX -> no displacement needed in mod 0
    decode_disp(buffer, 0, sizeof(buffer), &d);
    MCDC_CHECK(d.disp.is_valid != YES,
               "TC4 [Toggle C]: mod=0, sib.valid=YES, sib.base=0 -> no disp (C independence shown)");
}

/* ========================================================================= */
/* FUNCTION 4: find_instruction_reference() [Opcode Matching & Extensions]   */
/* Logic under test:                                                         */
/*   Two-byte opcode: if (b0 != eb0 || b1 != eb1) continue;                  */
/*   ModR/M extension: if (extension != -1 && reg != extension) continue;    */
/* ========================================================================= */
static void test_mcdc_find_instruction_reference(void) {
    printf("\n--- Test Suite 4: find_instruction_reference() [MC/DC Coverage] ---\n");
    printf("Decision: 2-byte opcode matching (b0 mismatch || b1 mismatch) & Opcode Extension\n");

    decoded_binary d;

    // Test 1: 2-byte Match Baseline (0x0F 0x84 = JE rel32)
    init_decoded(&d);
    d.opcode.size = TWO_B_OPCODE;
    d.opcode.bytes[0] = 0x0F;
    d.opcode.bytes[1] = 0x84;
    const instruction_reference_t *ref = find_instruction_reference(&d);
    MCDC_CHECK(ref != NULL && strcasecmp(ref->mnemonic_name, "je") == 0,
               "TC1 [2-Byte Match]: 0x0F 0x84 correctly matched to 'je'");

    // Test 2: Toggle Byte 1 (0x0F 0x85 = JNE rel32, not JE)
    // Demonstrates byte 1 mismatch independently rejects 'je'
    d.opcode.bytes[1] = 0x85;
    ref = find_instruction_reference(&d);
    MCDC_CHECK(ref != NULL && strcasecmp(ref->mnemonic_name, "jne") == 0,
               "TC2 [Toggle Byte 1]: 0x0F 0x85 correctly matched to 'jne' instead of 'je'");

    // Test 3: Opcode Extension /0 (0x83 with reg=0 -> ADD)
    init_decoded(&d);
    d.opcode.size = ONE_B_OPCODE;
    d.opcode.bytes[0] = 0x83;
    d.modrm.is_valid = YES;
    d.modrm.mod = 3;
    d.modrm.reg = 0; // /0 = ADD
    d.modrm.rm = 0;
    ref = find_instruction_reference(&d);
    MCDC_CHECK(ref != NULL && strcasecmp(ref->mnemonic_name, "add") == 0,
               "TC3 [Extension /0]: 0x83 with reg=0 correctly matched to 'add'");

    // Test 4: Toggle Opcode Extension from /0 to /5 (0x83 with reg=5 -> SUB)
    // Demonstrates reg extension independently switches mnemonic
    d.modrm.reg = 5; // /5 = SUB
    ref = find_instruction_reference(&d);
    MCDC_CHECK(ref != NULL && strcasecmp(ref->mnemonic_name, "sub") == 0,
               "TC4 [Toggle Extension to /5]: 0x83 with reg=5 correctly matched to 'sub'");

    // Test 5: Toggle Opcode Extension to /7 (0x83 with reg=7 -> CMP)
    d.modrm.reg = 7; // /7 = CMP
    ref = find_instruction_reference(&d);
    MCDC_CHECK(ref != NULL && strcasecmp(ref->mnemonic_name, "cmp") == 0,
               "TC5 [Toggle Extension to /7]: 0x83 with reg=7 correctly matched to 'cmp'");
}

/* ========================================================================= */
/* FUNCTION 5: decode_semantic() Guard Logic                                 */
/* Logic under test:                                                         */
/*   if (!d || !inst || d->length == 0) return -1;                           */
/*                                                                           */
/* Decision: A || B || C                                                     */
/* Conditions:                                                               */
/*   A: !d                                                                   */
/*   B: !inst                                                                */
/*   C: d->length == 0                                                       */
/* ========================================================================= */
static void test_mcdc_decode_semantic_guards(void) {
    printf("\n--- Test Suite 5: decode_semantic() Guards [MC/DC Coverage] ---\n");
    printf("Decision: if (!d || !inst || d->length == 0) -> return -1\n");

    decoded_binary d;
    instruction_reference_t inst;

    // Normal valid input
    init_decoded(&d);
    d.length = 1;
    d.opcode.size = ONE_B_OPCODE;
    d.opcode.bytes[0] = 0x90; // NOP

    // Test 1: Baseline Valid (A=False, B=False, C=False) -> returns 0
    MCDC_CHECK(decode_semantic(&d, 0x1000, &inst) == 0,
               "TC1 [Baseline Valid]: valid d, valid inst, length > 0 -> returns 0 (Success)");

    // Test 2: Toggle A (A=True [d == NULL], B=False, C=False) -> returns -1
    MCDC_CHECK(decode_semantic(NULL, 0x1000, &inst) == -1,
               "TC2 [Toggle A]: d == NULL -> returns -1 (A independence shown)");

    // Test 3: Toggle B (A=False, B=True [inst == NULL], C=False) -> returns -1
    MCDC_CHECK(decode_semantic(&d, 0x1000, NULL) == -1,
               "TC3 [Toggle B]: inst == NULL -> returns -1 (B independence shown)");

    // Test 4: Toggle C (A=False, B=False, C=True [length == 0]) -> returns -1
    d.length = 0;
    MCDC_CHECK(decode_semantic(&d, 0x1000, &inst) == -1,
               "TC4 [Toggle C]: d->length == 0 -> returns -1 (C independence shown)");
}

/* ========================================================================= */
/* FUNCTION 6: get_prefix() Classification                                   */
/* Tests each prefix group independently and validates non-prefix rejection. */
/* ========================================================================= */
static void test_mcdc_prefix_classification(void) {
    printf("\n--- Test Suite 6: get_prefix() Group Classification ---\n");

    prefix_info p;

    // Group 1: Lock (0xF0)
    p = get_prefix(0xF0);
    MCDC_CHECK(p.is_prefix == 1 && p.group == 1 && strcmp(p.name, "LOCK") == 0,
               "TC1 [Group 1]: 0xF0 classified as LOCK (Group 1)");

    // Group 2: Segment Override GS (0x65)
    p = get_prefix(0x65);
    MCDC_CHECK(p.is_prefix == 1 && p.group == 2 && strcmp(p.name, "GS") == 0,
               "TC2 [Group 2]: 0x65 classified as GS (Group 2)");

    // Group 3: Operand Size Override (0x66)
    p = get_prefix(0x66);
    MCDC_CHECK(p.is_prefix == 1 && p.group == 3 && strcmp(p.name, "OP_SIZE") == 0,
               "TC3 [Group 3]: 0x66 classified as OP_SIZE (Group 3)");

    // Group 4: Address Size Override (0x67)
    p = get_prefix(0x67);
    MCDC_CHECK(p.is_prefix == 1 && p.group == 4 && strcmp(p.name, "ADDR_SIZE") == 0,
               "TC4 [Group 4]: 0x67 classified as ADDR_SIZE (Group 4)");

    // Non-prefix byte (0x90 = NOP)
    p = get_prefix(0x90);
    MCDC_CHECK(p.is_prefix == 0,
               "TC5 [Non-Prefix]: 0x90 correctly classified as not a prefix (is_prefix = 0)");
}

/* ========================================================================= */
/* MAIN RUNNER                                                               */
/* ========================================================================= */
int main(void) {
    printf("====================================================================\n");
    printf("     DISASSEMBLER CORE UNIT TESTS (TARGETED MC/DC COVERAGE)        \n");
    printf("====================================================================\n");

    test_mcdc_needs_sib();
    test_mcdc_decode_immediate_f6_f7();
    test_mcdc_decode_disp();
    test_mcdc_find_instruction_reference();
    test_mcdc_decode_semantic_guards();
    test_mcdc_prefix_classification();

    printf("\n====================================================================\n");
    printf("                      FINAL VERDICT                                 \n");
    printf("====================================================================\n");
    printf(" Total Tests Run    : %d\n", g_tests_run);
    printf(" Passed Conditions  : %d\n", g_tests_passed);
    printf(" Failed Conditions  : %d\n", g_tests_failed);

    if (g_tests_failed == 0) {
        printf(" Verdict            : [PASSED] 100%% of MC/DC condition test pairs passed.\n");
        printf("====================================================================\n\n");
        return 0;
    } else {
        printf(" Verdict            : [FAILED] %d test condition(s) failed.\n", g_tests_failed);
        printf("====================================================================\n\n");
        return 1;
    }
}
