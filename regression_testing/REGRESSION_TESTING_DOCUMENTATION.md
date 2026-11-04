# Regression Testing & Feature Expansion Documentation

**Branch:** `ragini`  
**Folder:** `regression_testing/`  
**Author:** Ragini  
**Baseline Test Harness:** Atharva's MC/DC 97-check and State verification suites  

---

## 1. What is Regression Testing?

Regression testing verifies that **new feature additions or modifications do NOT break existing functionality**.

$$\text{Existing Working Code} + \text{New Feature} \xrightarrow{\text{Run Baseline Tests}} \text{All Tests Still Pass} \implies \text{No Regression}$$

- **Baseline:** Atharva's 97 MC/DC unit tests and 11 instruction decoder state checks all passed.
- **Change:** Added `CLC` instruction to table, added `decoder_has_memory_operand()` to decoder core.
- **Proof:** Re-ran unit tests (97/97 passed), re-ran state tests (12/12 passed + memory hook verified).

---

## 2. Summary of Atharva's Baseline Files

| File | Purpose | Status in `regression_testing/` |
|---|---|---|
| `prefix.c` / `prefix.h` | LOCK / REP / 0x66 / 0x67 / segment overrides | Preserved baseline |
| `decoder.c` / `decoder.h` | Slices opcode, ModR/M, SIB, displacement, immediate | Enhanced with `decoder_has_memory_operand()` |
| `instruction_table.c` / `instruction_table.h` | Maps raw opcodes to mnemonics & operands | Enhanced with `CLC` (0xF8) |
| `semantic_decoder.c` / `semantic_decoder.h` | Resolves operands, registers, addressing | Preserved baseline |
| `test_mcdc.c` | 97 MC/DC coverage verification unit tests | Cleaned format, 97/97 passing |
| `test_state.c` | Real instruction encoding verification suite | Enhanced with CLC & memory hook tests |
| `adapter.c` / `formatter.c` | Debugger integration & AT&T formatting | Preserved baseline |
| `Makefile` | Build automation targets | Preserved baseline |

---

## 3. Code Modifications (Lines Above & Below)

### Change 1: `instruction_table.c` (Adding `CLC` opcode `0xF8`)
**Location:** Lines 104–115

```c
// [LINES ABOVE]
    // CDQ (No operands)
    { .opcode_bytes = {0x99}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "CDQ", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ZERO },

// [FEATURE ADDITION - RAGINI]
    /***************************************************************************
     * [FEATURE ADDITION 1 - RAGINI]
     * INSTRUCTION: CLC (Clear Carry Flag)
     * OPCODE     : 0xF8 (1-byte opcode, no operands)
     * PURPOSE    : Implements CLC in instruction lookup table.
     *              Verified via regression testing to prove table extensibility.
     ***************************************************************************/
    { .opcode_bytes = {0xF8}, .opcode_size = ONE_B_OPCODE, .mnemonic_name = "CLC", .modrm_needed = false, .opcode_extension = OPCODE_EXTENSION_R, .operand_count = COUNT_ZERO },

// [LINES BELOW]
    // =========================================================================
    // 4. LOGIC: AND, OR, XOR, NOT, TEST
    // =========================================================================
```

---

### Change 2: `decoder.h` & `decoder.c` (Adding `decoder_has_memory_operand()`)
**Purpose:** Future scope for the Step Debugger. Inspects decoded physical fields (`ModR/M mod != 3`, `SIB`, direct `moffs`) to determine if an instruction accesses memory. This enables debugger memory watchpoints and bus interception without overhead.

#### In `decoder.h` (Lines 15–30):
```c
// [LINES ABOVE]
int decode_immediate(uint8_t *bytes, int pos, int size, decoded_binary *d);
int decode_instruction(uint8_t *bytes, int pos, int size, decoded_binary *d);
void print_decoded(decoded_binary *d);

// [FEATURE ADDITION - RAGINI]
/***************************************************************************
 * [FEATURE ADDITION 2 - RAGINI]
 * FUTURE SCOPE FOR STEP DEBUGGER:
 * Function: decoder_has_memory_operand
 * Purpose : Inspects physical decoding fields (ModR/M, SIB, Displacement)
 *           to check if an instruction reads or writes memory.
 * Benefits: Allows debugger to intercept memory watchpoints and evaluate
 *           effective addresses before memory execution.
 * Returns : 1 if memory operand present, 0 if register/immediate only.
 ***************************************************************************/
int decoder_has_memory_operand(const decoded_binary *d);

// [LINES BELOW]
#endif
```

#### In `decoder.c` (Lines 430–462):
```c
// [LINES ABOVE]
    for (i = 0; i < count; i++) {
      printf(" %02X", d->immediate.bytes[i]);
    }
    printf("\n");
  }
}

// [FEATURE ADDITION - RAGINI]
/***************************************************************************
 * [FEATURE ADDITION 2 - RAGINI]
 * FUTURE SCOPE: STEP DEBUGGER MEMORY WATCHPOINT HOOK
 * Function: decoder_has_memory_operand
 * Details :
 *   1. If ModR/M is present and mod != 3 (i.e. mod 0, 1, 2), operand refers to memory.
 *   2. If SIB is valid or displacement is active with mod != 3, memory is accessed.
 *   3. If opcode is direct memory offset (0xA0..0xA3: MOV AL/EAX, [moffs]), memory is accessed.
 *   4. Returns 1 for memory operands, 0 for register-only or immediate instructions.
 ***************************************************************************/
int decoder_has_memory_operand(const decoded_binary *d) {
  if (!d || d->length == 0) {
    return 0;
  }
  // ModR/M mod 0, 1, 2 indicate memory references ([reg], [reg+disp], etc.)
  if (d->modrm.is_valid == YES && d->modrm.mod != 3) {
    return 1;
  }
  // SIB byte presence indicates scaled index memory addressing
  if (d->sib.is_valid == YES) {
    return 1;
  }
  // Direct memory offset moves (MOV AL/AX/EAX, moffs)
  if (d->opcode.size == ONE_B_OPCODE &&
      (d->opcode.bytes[0] >= 0xA0 && d->opcode.bytes[0] <= 0xA3)) {
    return 1;
  }
  return 0;
}
```

---

### Change 3: `test_state.c` (Testing Both Features & Clean Output)
**Location:** Lines 55–80

```c
// [LINES ABOVE]
    uint8_t t11[] = {0x67, 0x8B, 0x00};
    run_test("MOV (%bx,%si), %eax (0x67)", t11, sizeof(t11), 0x0804801D);

// [FEATURE ADDITION - RAGINI]
    /***************************************************************************
     * [FEATURE ADDITION 1 - RAGINI]
     * Verification test for added instruction: CLC (opcode 0xF8)
     ***************************************************************************/
    uint8_t t12[] = {0xF8};
    run_test("CLC", t12, sizeof(t12), 0x08048020);

    /***************************************************************************
     * [FEATURE ADDITION 2 - RAGINI]
     * Verification test for decoder feature: decoder_has_memory_operand()
     ***************************************************************************/
    printf("\nDecoder Feature Verification: decoder_has_memory_operand()\n");
    decoded_binary d_reg, d_mem;
    decode_instruction(t2, 0, sizeof(t2), &d_reg); // MOV %esp, %ebp (register only)
    decode_instruction(t4, 0, sizeof(t4), &d_mem); // LEA 0x8(%ebp), %eax (memory reference)
    int mem_check_pass = (decoder_has_memory_operand(&d_reg) == 0 && decoder_has_memory_operand(&d_mem) == 1);
    if (mem_check_pass) {
        printf("[PASS] Memory Reference Hook: Register-only -> 0, Memory operand -> 1\n");
    } else {
        printf("[FAIL] Memory Reference Hook verification failed\n");
    }

// [LINES BELOW]
    printf("\nAll test cases successfully decoded!\n\n");
    return 0;
}
```

---

## 4. How to Run on Linux Terminal (Step-by-Step)

Open your Linux / WSL / Git Bash terminal and run:

```bash
# 1. Switch to your project directory
cd "/path/to/Disassembler_and_Step_Debugger"

# 2. Check out your branch
git checkout ragini

# 3. Enter the regression testing directory
cd regression_testing

# 4. Clean any prior build files
make clean

# 5. Run the 97 MC/DC Unit Tests (No '===' lines, clean case list)
make unit_tests

# 6. Run the Decoder State & Feature Verification Suite
make test_state

# 7. Run Complete Regression Suite (runs all suites together)
make test
```

---

## 5. Where & When to Take Screenshots (For Viva / Submission)

### Screenshot 1: MC/DC Unit Test Results (97/97 Passed)
- **When:** Right after executing `make unit_tests`.
- **Where to capture:** Scroll to the bottom of the terminal output showing:
```
case 95 - "TC2: 0x66 MOV AX,imm16 -> destination width=16 (demoted)" passed
case 96 - "TC1: 1-byte instruction -> accepted (len=1)" passed
case 97 - "TC2: empty buffer -> returns 0" passed

RESULTS
Total : 97
Passed: 97
Failed: 0
Verdict: ALL PASSED -- MC/DC coverage satisfied.
```
- **What this proves:** The original 97 unit test cases all pass without any regressions.

---

### Screenshot 2: Decoder State & Feature Verification
- **When:** Right after executing `make test_state`.
- **Where to capture:** The terminal screen showing:
```
Current Decoder State Verification Suite

[PASS] PUSH %ebp                 -> Mnemonic: PUSH   (Length: 1 bytes, Operands: 1)
[PASS] MOV %esp, %ebp            -> Mnemonic: MOV    (Length: 2 bytes, Operands: 2)
[PASS] SUB $0x10, %esp           -> Mnemonic: SUB    (Length: 3 bytes, Operands: 2)
[PASS] LEA 0x8(%ebp), %eax       -> Mnemonic: LEA    (Length: 3 bytes, Operands: 2)
[PASS] IDIV %ecx                 -> Mnemonic: IDIV   (Length: 2 bytes, Operands: 1)
[PASS] JE rel32                  -> Mnemonic: JE     (Length: 6 bytes, Operands: 1)
[PASS] RET                       -> Mnemonic: RET    (Length: 1 bytes, Operands: 0)
[PASS] MOV $0x1234, %ax (0x66)   -> Mnemonic: MOV    (Length: 4 bytes, Operands: 2)
[PASS] LOCK ADD $0x1, (%eax)     -> Mnemonic: ADD    (Length: 4 bytes, Operands: 2)
[PASS] MOV %gs:(%eax), %edx      -> Mnemonic: MOV    (Length: 3 bytes, Operands: 2)
[PASS] MOV (%bx,%si), %eax (0x67) -> Mnemonic: MOV    (Length: 3 bytes, Operands: 2)
[PASS] CLC                       -> Mnemonic: CLC    (Length: 1 bytes, Operands: 0)

Decoder Feature Verification: decoder_has_memory_operand()
[PASS] Memory Reference Hook: Register-only -> 0, Memory operand -> 1

All test cases successfully decoded!
```
- **What this proves:**
  1. The 11 original instruction decodings still pass.
  2. The new `CLC` instruction decodes with 1 byte and 0 operands.
  3. The new `decoder_has_memory_operand()` accurately differentiates register vs. memory instructions.

---

### Screenshot 3: Full Regression Proof
- **When:** Right after executing `make test`.
- **Where to capture:** Bottom line:
```
REGRESSION PASS: unit_tests + test_state + test_adapter_bridge
```
- **What this proves:** End-to-end regression proof across unit tests, state verification, and debugger adapter bridge.
