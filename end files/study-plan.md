# x86-32 ELF Disassembler & Interactive Debugger — Study Plan (C)

> **Target:** IA-32 only · Language: C · Output: `objdump -d` AT&T format · Fully interactive ptrace debugger
>
> SETcc/MOVZX deferred as stretch goal (Phase 3.5)

---

## Phase 0 — Foundations (Days 1–3)

**Goal:** Understand how a CPU sees machine code and how Linux processes work.

### 0.1 Number Systems & Bit Manipulation
- Binary, hexadecimal, two's complement (signed integers)
- Bitwise AND, OR, XOR, shift, mask — you'll use these constantly in C for decoding
- **Little-endian** byte order: `0x12345678` is stored as `78 56 34 12` in memory

### 0.2 x86-32 Register Model

```
General Purpose (32-bit → 16-bit → 8-bit):
┌─────────────────────────────────────────┐
│ EAX  │  AX  │  AH │ AL │  Accumulator  │
│ EBX  │  BX  │  BH │ BL │  Base         │
│ ECX  │  CX  │  CH │ CL │  Counter      │
│ EDX  │  DX  │  DH │ DL │  Data         │
│ ESI  │  SI  │     │    │  Source Index  │
│ EDI  │  DI  │     │    │  Dest Index   │
│ EBP  │  BP  │     │    │  Base Pointer │
│ ESP  │  SP  │     │    │  Stack Pointer│
└─────────────────────────────────────────┘

Register encoding (3-bit index used in ModRM/SIB):
  000=EAX  001=ECX  010=EDX  011=EBX
  100=ESP  101=EBP  110=ESI  111=EDI
```

### 0.3 Stack & Calling Convention (cdecl)

```
High addresses
┌──────────────┐
│   arg2       │  [EBP+12]
│   arg1       │  [EBP+8]
│ return addr  │  [EBP+4]   ← CALL pushed this
│ saved EBP    │  [EBP]     ← PUSH %ebp; MOV %esp,%ebp
│ local1       │  [EBP-4]
│ local2       │  [EBP-8]   ← SUB $0x10,%esp made room
└──────────────┘
Low addresses (ESP points here)
```

### 0.4 EFLAGS (needed for Jcc/SETcc)

| Flag | Bit | Set when |
|------|-----|----------|
| CF (Carry) | 0 | Unsigned overflow / borrow |
| ZF (Zero) | 6 | Result is zero |
| SF (Sign) | 7 | Result is negative (MSB=1) |
| OF (Overflow) | 11 | Signed overflow |

- `JE/JZ` → ZF=1, `JNE/JNZ` → ZF=0
- `JG` → ZF=0 AND SF=OF, `JL` → SF≠OF
- `JA` → CF=0 AND ZF=0 (unsigned above), `JB` → CF=1 (unsigned below)

### 📚 Materials
- **Intel SDM Vol.1 Ch.3** — Basic Execution Environment: [Intel SDM](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- **"Programming from the Ground Up" by Jonathan Bartlett** — Free, great x86+Linux intro
- **CS:APP Ch.3** — Machine-Level Representation of Programs (Bryant & O'Hallaron)

---

## Phase 1 — ELF Parsing in C (Days 4–7)

**Goal:** Read a 32-bit ELF binary, extract `.text` bytes and the symbol table.

### 1.1 ELF Structure

```
┌──────────────────┐  offset 0
│   ELF Header     │  e_ident, e_entry, e_shoff, e_phoff
├──────────────────┤
│ Program Headers  │  Segments (for loader — you mostly skip these)
├──────────────────┤
│   .text          │  ← Machine code lives here
│   .data / .rodata│
│   .symtab        │  ← Symbol table (function names + addresses)
│   .strtab        │  ← String table (names for symbols)
│   .shstrtab      │  ← String table (names for sections)
├──────────────────┤
│ Section Headers  │  Describes each section
└──────────────────┘
```

### 1.2 C Structs — Use `<elf.h>` Directly

```c
#include <elf.h>

// Key structs (already defined in elf.h):
// Elf32_Ehdr  — ELF header
// Elf32_Shdr  — Section header
// Elf32_Sym   — Symbol table entry

// Fields you need:
// Elf32_Ehdr: e_ident, e_type, e_machine (==EM_386), e_entry,
//             e_shoff, e_shnum, e_shstrndx
// Elf32_Shdr: sh_name, sh_type, sh_flags, sh_addr, sh_offset, sh_size
// Elf32_Sym:  st_name, st_value, st_size, st_info (ELF32_ST_TYPE==STT_FUNC)
```

### 1.3 Implementation Steps

```c
// 1. mmap the entire file (simplest approach)
int fd = open(argv[1], O_RDONLY);
struct stat st;
fstat(fd, &st);
uint8_t *file = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

// 2. Validate ELF header
Elf32_Ehdr *ehdr = (Elf32_Ehdr *)file;
assert(memcmp(ehdr->e_ident, ELFMAG, 4) == 0);  // "\x7fELF"
assert(ehdr->e_ident[EI_CLASS] == ELFCLASS32);    // 32-bit
assert(ehdr->e_machine == EM_386);                 // x86

// 3. Find section headers
Elf32_Shdr *shdrs = (Elf32_Shdr *)(file + ehdr->e_shoff);
char *shstrtab = (char *)(file + shdrs[ehdr->e_shstrndx].sh_offset);

// 4. Find .text section
for (int i = 0; i < ehdr->e_shnum; i++) {
    char *name = shstrtab + shdrs[i].sh_name;
    if (strcmp(name, ".text") == 0) {
        uint8_t *text_bytes = file + shdrs[i].sh_offset;
        uint32_t text_addr  = shdrs[i].sh_addr;
        uint32_t text_size  = shdrs[i].sh_size;
        // Feed to disassembler
    }
}

// 5. Find .symtab + .strtab → build address→name lookup
```

### 1.4 Testing

```bash
# Create a test binary
echo 'int main() { return 42; }' > test.c
gcc -m32 -O0 -fno-pie -no-pie -o test32 test.c

# Verify with system tools
readelf -h test32     # ELF header
readelf -S test32     # Section headers
readelf -s test32     # Symbol table
objdump -d test32     # Reference disassembly
```

### 📚 Materials
- **`man 5 elf`** — Quick reference for all ELF structs
- **ELF Specification (TIS):** [refspecs.linuxbase.org/elf/elf.pdf](http://refspecs.linuxbase.org/elf/elf.pdf)
- **`/usr/include/elf.h`** — Read it! It *is* your API
- **"Learning Linux Binary Analysis" by Ryan O'Neill** — Ch.1-2

---

## Phase 2 — x86 Instruction Encoding (Days 8–14) ⭐ CRITICAL

**Goal:** Deeply understand how x86 bytes map to instructions. This is the hardest phase.

> [!IMPORTANT]
> Spend time **hand-decoding** hex with pencil and paper before writing code. If you understand ModRM/SIB, Phases 3–5 become mechanical.

### 2.1 Instruction Byte Layout

```
┌──────────┬────────┬───────┬─────┬──────────────┬─────────────┐
│ Prefixes │ Opcode │ ModRM │ SIB │ Displacement │ Immediate   │
│ (0-4 B)  │ (1-2B) │ (0-1B)│(0-1)│ (0,1,2,4 B)  │ (0,1,2,4 B) │
└──────────┴────────┴───────┴─────┴──────────────┴─────────────┘
```

### 2.2 ModR/M Byte — THE KEY TO EVERYTHING

```
  7  6 │ 5  4  3 │ 2  1  0
 ┌─────┬─────────┬─────────┐
 │ mod │   reg   │   r/m   │
 └─────┴─────────┴─────────┘
```

**mod field:**

| mod | Meaning |
|-----|---------|
| `00` | `[r/m]` — memory, no displacement (special cases: r/m=101 → `[disp32]`, r/m=100 → SIB) |
| `01` | `[r/m + disp8]` — memory + signed 8-bit displacement |
| `10` | `[r/m + disp32]` — memory + 32-bit displacement |
| `11` | Register-to-register, no memory |

**reg field:** Either a register operand OR an opcode extension (`/0`..`/7`), depending on the instruction.

**r/m field (when mod ≠ 11):**

| r/m | Register | Note |
|-----|----------|------|
| 000 | EAX | |
| 001 | ECX | |
| 010 | EDX | |
| 011 | EBX | |
| 100 | — | **SIB byte follows** |
| 101 | EBP | But if mod=00, means `[disp32]` instead |
| 110 | ESI | |
| 111 | EDI | |

### 2.3 SIB Byte (Scale-Index-Base)

```
  7  6 │ 5  4  3 │ 2  1  0
 ┌─────┬─────────┬─────────┐
 │scale│  index  │  base   │
 └─────┴─────────┴─────────┘

address = base_reg + (index_reg × 2^scale) + displacement
```

- Scale: `00`=×1, `01`=×2, `10`=×4, `11`=×8
- Index = `100` (ESP) means **no index** (ESP can't be an index register)
- Base = `101` with mod=`00` means **disp32 only** (no base register)

### 2.4 Prefix Bytes

| Prefix | Hex | What It Does |
|--------|-----|--------------|
| Operand-size override | `0x66` | 32-bit → 16-bit operands (`movl` → `movw`) |
| Address-size override | `0x67` | 32-bit → 16-bit addressing (rare) |
| Segment overrides | `0x26,0x2E,0x36,0x3E,0x64,0x65` | Mostly ignorable |

### 2.5 Two-Byte Opcodes (`0x0F` escape)

When first byte is `0x0F`, the *second* byte is the real opcode. Your instructions using this:
- `IMUL r32,r/m32` → `0F AF`
- `Jcc rel32` → `0F 8x` (84=JE, 85=JNE, 87=JA, 82=JB, 8C=JL, 8D=JGE, 8E=JLE, 8F=JG)
- `SETcc` → `0F 9x`
- `MOVZX` → `0F B6/B7`
- Multi-byte NOP → `0F 1F`

### 2.6 Complete Opcode Map for Your Instruction Set

#### Data Movement

| Instruction | Opcode | Encoding | Notes |
|-------------|--------|----------|-------|
| `MOV r/m32, r32` | `89 /r` | ModRM: reg→r/m | |
| `MOV r32, r/m32` | `8B /r` | ModRM: r/m→reg | |
| `MOV r/m32, imm32` | `C7 /0` | reg field=0, +imm32 | |
| `MOV r32, imm32` | `B8+rd` | Opcode B8–BF, reg in low 3 bits | No ModRM! |
| `MOV r/m8, r8` | `88 /r` | 8-bit | |
| `MOV r8, r/m8` | `8A /r` | 8-bit | |
| `MOV r/m8, imm8` | `C6 /0` | 8-bit | |
| `LEA r32, m` | `8D /r` | Always memory source | |
| `PUSH r32` | `50+rd` | 50–57, reg in low 3 bits | No ModRM! |
| `PUSH r/m32` | `FF /6` | reg field=6 | |
| `PUSH imm32` | `68` | +imm32 | |
| `PUSH imm8` | `6A` | +imm8 (sign-extended) | |
| `POP r32` | `58+rd` | 58–5F | No ModRM! |
| `POP r/m32` | `8F /0` | reg field=0 | |

#### Arithmetic

| Instruction | Opcode | Encoding | Notes |
|-------------|--------|----------|-------|
| `ADD r/m32, r32` | `01 /r` | | |
| `ADD r32, r/m32` | `03 /r` | | |
| `ADD EAX, imm32` | `05` | Short form, no ModRM | |
| `ADD r/m32, imm32` | `81 /0` | reg=0 | |
| `ADD r/m32, imm8` | `83 /0` | Sign-extended | |
| `SUB r/m32, r32` | `29 /r` | | |
| `SUB r32, r/m32` | `2B /r` | | |
| `SUB EAX, imm32` | `2D` | Short form | |
| `SUB r/m32, imm32` | `81 /5` | reg=5 | |
| `SUB r/m32, imm8` | `83 /5` | Sign-extended | |
| `INC r32` | `40+rd` | 40–47 | No ModRM! |
| `INC r/m32` | `FF /0` | reg=0 | |
| `DEC r32` | `48+rd` | 48–4F | No ModRM! |
| `DEC r/m32` | `FF /1` | reg=1 | |
| `IMUL r32, r/m32` | `0F AF /r` | Two-byte opcode | |
| `IMUL r32, r/m32, imm32` | `69 /r` | Three-operand + imm32 | |
| `IMUL r32, r/m32, imm8` | `6B /r` | Three-operand + imm8 | |
| `NEG r/m32` | `F7 /3` | reg=3 | |

#### Division

| Instruction | Opcode | Encoding | Notes |
|-------------|--------|----------|-------|
| `DIV r/m32` | `F7 /6` | Unsigned EDX:EAX ÷ operand | |
| `IDIV r/m32` | `F7 /7` | Signed EDX:EAX ÷ operand | |
| `CDQ` | `99` | Sign-extend EAX → EDX:EAX | Single byte, no operands |

#### Logic

| Instruction | Opcode | Encoding | Notes |
|-------------|--------|----------|-------|
| `AND r/m32, r32` | `21 /r` | | |
| `AND r32, r/m32` | `23 /r` | | |
| `AND r/m32, imm32` | `81 /4` | | |
| `AND r/m32, imm8` | `83 /4` | | |
| `OR r/m32, r32` | `09 /r` | | |
| `OR r32, r/m32` | `0B /r` | | |
| `OR r/m32, imm32` | `81 /1` | | |
| `OR r/m32, imm8` | `83 /1` | | |
| `XOR r/m32, r32` | `31 /r` | | |
| `XOR r32, r/m32` | `33 /r` | | |
| `XOR r/m32, imm32` | `81 /6` | | |
| `XOR r/m32, imm8` | `83 /6` | | |
| `NOT r/m32` | `F7 /2` | | |
| `TEST r/m32, r32` | `85 /r` | | |
| `TEST EAX, imm32` | `A9` | Short form | |
| `TEST r/m32, imm32` | `F7 /0` | | |

#### Comparison

| Instruction | Opcode | Encoding | Notes |
|-------------|--------|----------|-------|
| `CMP r/m32, r32` | `39 /r` | | |
| `CMP r32, r/m32` | `3B /r` | | |
| `CMP EAX, imm32` | `3D` | Short form | |
| `CMP r/m32, imm32` | `81 /7` | reg=7 | |
| `CMP r/m32, imm8` | `83 /7` | Sign-extended | |

#### Control Flow

| Instruction | Opcode | Encoding | Notes |
|-------------|--------|----------|-------|
| `JMP rel8` | `EB` | +signed byte | Target = EIP + 2 + rel8 |
| `JMP rel32` | `E9` | +signed dword | Target = EIP + 5 + rel32 |
| `JE/JZ rel8` | `74` | | |
| `JE/JZ rel32` | `0F 84` | | |
| `JNE/JNZ rel8` | `75` | | |
| `JNE/JNZ rel32` | `0F 85` | | |
| `JG rel8` | `7F` | | |
| `JG rel32` | `0F 8F` | | |
| `JL rel8` | `7C` | | |
| `JL rel32` | `0F 8C` | | |
| `JGE rel8` | `7D` | | |
| `JGE rel32` | `0F 8D` | | |
| `JLE rel8` | `7E` | | |
| `JLE rel32` | `0F 8E` | | |
| `JA rel8` | `77` | | Unsigned above |
| `JA rel32` | `0F 87` | | |
| `JB rel8` | `72` | | Unsigned below |
| `JB rel32` | `0F 82` | | |
| `CALL rel32` | `E8` | +signed dword | Target = EIP + 5 + rel32 |
| `CALL r/m32` | `FF /2` | Indirect call | |
| `RET` | `C3` | | Single byte |
| `LEAVE` | `C9` | | Single byte |

#### Shift/Rotate

| Instruction | Opcode | Encoding | Notes |
|-------------|--------|----------|-------|
| `SHL r/m32, 1` | `D1 /4` | | |
| `SHL r/m32, CL` | `D3 /4` | | |
| `SHL r/m32, imm8` | `C1 /4` | | |
| `SHR r/m32, 1` | `D1 /5` | | |
| `SHR r/m32, CL` | `D3 /5` | | |
| `SHR r/m32, imm8` | `C1 /5` | | |
| `SAR r/m32, 1` | `D1 /7` | | |
| `SAR r/m32, CL` | `D3 /7` | | |
| `SAR r/m32, imm8` | `C1 /7` | | |

#### Miscellaneous

| Instruction | Opcode | Encoding | Notes |
|-------------|--------|----------|-------|
| `NOP` | `90` | | Actually `XCHG EAX,EAX` |
| Multi-byte NOP | `0F 1F /0` | + ModRM | Padding NOPs (various lengths) |

### 2.7 Hand-Decode Exercise

Before writing code, decode these by hand:

```
55                    → ?
89 e5                 → ?
83 ec 10              → ?
8b 45 08              → ?
3b 45 0c              → ?
7e 07                 → ?
c7 45 fc 00 00 00 00  → ?
e8 xx xx xx xx        → ?
```

<details>
<summary>Answers</summary>

```
55                    → push   %ebp           (PUSH r32: 50+rd, rd=5=EBP)
89 e5                 → mov    %esp,%ebp      (MOV r/m32,r32: 89, ModRM=e5→mod=11,reg=100=ESP,r/m=101=EBP)
83 ec 10              → sub    $0x10,%esp     (SUB r/m32,imm8: 83 /5, ModRM=ec→mod=11,reg=5,r/m=100=ESP, imm8=0x10)
8b 45 08              → mov    0x8(%ebp),%eax (MOV r32,r/m32: 8B, ModRM=45→mod=01,reg=000=EAX,r/m=101=EBP, disp8=0x08)
3b 45 0c              → cmp    0xc(%ebp),%eax (CMP r32,r/m32: 3B, ModRM=45→mod=01,reg=000=EAX,r/m=101=EBP, disp8=0x0c)
7e 07                 → jle    <addr+9>       (JLE rel8: 7E, offset=0x07, target=current_addr+2+7)
c7 45 fc 00 00 00 00  → movl   $0x0,-0x4(%ebp)(MOV r/m32,imm32: C7 /0, ModRM=45→mod=01,r/m=EBP, disp=-4, imm=0)
e8 xx xx xx xx        → call   <target>       (CALL rel32)
```

</details>

### 📚 Materials
- **Intel SDM Vol.2 Ch.2** — Instruction Format (THE reference for encoding)
- **Intel SDM Vol.2 Appendix A** — Opcode map tables
- **[ref.x86asm.net/coder32.html](http://ref.x86asm.net/coder32.html)** — Searchable one-page opcode table
- **[wiki.osdev.org/X86-64_Instruction_Encoding](https://wiki.osdev.org/X86-64_Instruction_Encoding)** — Great ModRM/SIB visuals
- **[sandpile.org/x86/opc_1.htm](https://sandpile.org/x86/opc_1.htm)** — Compact opcode maps

---

## Phase 3 — Disassembler Implementation in C (Days 15–24)

**Goal:** Build a linear-sweep disassembler that outputs objdump-format AT&T syntax.

### 3.1 C Project Structure

```
disasm/
├── Makefile
├── include/
│   ├── elf_parser.h       # ELF reading API
│   ├── decoder.h          # Instruction decoder API
│   ├── opcode_table.h     # Opcode lookup table
│   ├── formatter.h        # AT&T syntax formatter
│   └── types.h            # Shared types & enums
├── src/
│   ├── main.c             # CLI entry point
│   ├── elf_parser.c       # ELF header/section/symbol parsing
│   ├── decoder.c          # ModRM/SIB decode + instruction decode loop
│   ├── opcode_table.c     # Hardcoded opcode → instruction mapping
│   └── formatter.c        # Render decoded instruction as AT&T string
└── tests/
    ├── test_arith.c       # Test programs to compile with gcc -m32
    ├── test_control.c
    ├── test_fib.c
    └── run_tests.sh       # Diff your output against objdump
```

### 3.2 Core C Data Structures

```c
// types.h

typedef enum {
    OP_NONE,          // No operand (RET, LEAVE, CDQ, NOP)
    OP_REG,           // Register: %eax
    OP_MEM,           // Memory: disp(%base,%index,scale)
    OP_IMM,           // Immediate: $0x42
    OP_REL,           // Relative address (jumps/calls): 0x8049010
} operand_type_t;

typedef enum {
    SIZE_8  = 1,
    SIZE_16 = 2,
    SIZE_32 = 4,
} operand_size_t;

typedef struct {
    operand_type_t type;
    operand_size_t size;
    union {
        uint8_t reg;                // Register index (0-7)
        struct {                    // Memory operand
            int8_t  base;           // Base register (-1 = none)
            int8_t  index;          // Index register (-1 = none)
            uint8_t scale;          // 1, 2, 4, or 8
            int32_t disp;           // Displacement
            bool    has_disp;       // Explicit disp even if 0?
        } mem;
        int32_t imm;                // Immediate value
        uint32_t rel_addr;          // Absolute target address (for jumps)
    };
} operand_t;

typedef struct {
    uint32_t address;               // Virtual address of this instruction
    uint8_t  bytes[15];             // Raw bytes (x86 max = 15)
    uint8_t  length;                // Instruction length in bytes
    char     mnemonic[16];          // "mov", "add", "jle", etc.
    operand_t operands[3];          // Up to 3 operands (IMUL r,r/m,imm)
    uint8_t  num_operands;
    bool     has_prefix_66;         // Operand-size override active?
    bool     is_valid;              // Successfully decoded?
} instruction_t;
```

### 3.3 Opcode Table Design

```c
// opcode_table.h

typedef enum {
    ENC_NONE,          // No operands (RET, NOP, CDQ, LEAVE)
    ENC_MODRM,         // Standard ModRM encoding
    ENC_MODRM_EXT,     // ModRM with reg field as opcode extension (/0../7)
    ENC_PLUS_REG,      // Register encoded in low 3 bits of opcode
    ENC_IMM8,          // Opcode + imm8 (PUSH imm8, JMP rel8)
    ENC_IMM32,         // Opcode + imm32 (PUSH imm32, CALL rel32)
    ENC_AL_IMM,        // Short form: AL/AX/EAX, imm (ADD EAX, imm32)
} encoding_type_t;

typedef struct {
    uint16_t opcode;        // 1 byte, or 0x0Fxx for two-byte
    uint8_t  ext;           // Opcode extension (/0../7), or 0xFF if unused
    const char *mnemonic;
    encoding_type_t encoding;
    uint8_t  num_operands;
    operand_type_t op1_type, op2_type;
    operand_size_t op_size;
    // ... flags for direction bit, sign-extend, etc.
} opcode_entry_t;

// Lookup function:
const opcode_entry_t *lookup_opcode(uint8_t byte1, uint8_t byte2, uint8_t modrm_reg);
```

> [!TIP]
> **Implementation approach:** Use a 256-entry array for single-byte opcodes (direct index by first byte). For two-byte opcodes (`0x0F xx`), use a second 256-entry array indexed by the second byte. For opcodes that use the ModRM reg field as extension (`81 /0`..`81 /7`), do a secondary lookup by reg field.

### 3.4 Decoder Algorithm

```c
// decoder.c — core loop

int decode_instruction(const uint8_t *code, uint32_t addr, instruction_t *inst) {
    memset(inst, 0, sizeof(*inst));
    inst->address = addr;
    int offset = 0;

    // 1. Consume prefix bytes
    while (is_prefix(code[offset])) {
        if (code[offset] == 0x66) inst->has_prefix_66 = true;
        offset++;
    }

    // 2. Read opcode
    uint8_t opcode = code[offset++];
    bool two_byte = false;
    uint8_t opcode2 = 0;
    if (opcode == 0x0F) {
        two_byte = true;
        opcode2 = code[offset++];
    }

    // 3. Peek at ModRM if present (to get reg extension)
    uint8_t modrm = code[offset];  // Don't consume yet
    uint8_t modrm_reg = (modrm >> 3) & 0x07;

    // 4. Look up in opcode table
    const opcode_entry_t *entry = lookup_opcode(opcode, opcode2, modrm_reg);
    if (!entry) {
        inst->is_valid = false;
        inst->length = 1;
        return 1;  // Unknown opcode
    }

    strcpy(inst->mnemonic, entry->mnemonic);

    // 5. Decode operands based on encoding type
    switch (entry->encoding) {
        case ENC_MODRM:
        case ENC_MODRM_EXT:
            offset += decode_modrm(code + offset, inst, entry);
            break;
        case ENC_PLUS_REG:
            inst->operands[0].type = OP_REG;
            inst->operands[0].reg = opcode & 0x07;  // low 3 bits
            inst->num_operands = 1;
            break;
        case ENC_IMM8:
            // read 1 byte immediate or rel8
            break;
        case ENC_IMM32:
            // read 4 byte immediate or rel32
            break;
        // ...
    }

    inst->length = offset;
    memcpy(inst->bytes, code, inst->length);
    inst->is_valid = true;
    return inst->length;
}
```

### 3.5 ModRM/SIB Decoder

```c
// The most important function in your entire project

int decode_modrm(const uint8_t *bytes, instruction_t *inst, const opcode_entry_t *entry) {
    int offset = 0;
    uint8_t modrm = bytes[offset++];

    uint8_t mod = (modrm >> 6) & 0x03;
    uint8_t reg = (modrm >> 3) & 0x07;
    uint8_t rm  = modrm & 0x07;

    // Register operand (from reg field)
    // ... set based on entry->encoding (might be opcode ext, not operand)

    if (mod == 0x03) {
        // Register-to-register: rm is a register
        // operand.type = OP_REG, operand.reg = rm
    } else {
        // Memory operand
        operand_t *mem_op = &inst->operands[...];
        mem_op->type = OP_MEM;
        mem_op->mem.base = rm;
        mem_op->mem.index = -1;
        mem_op->mem.scale = 1;

        // Special case: rm=100 → SIB follows
        if (rm == 4) {
            uint8_t sib = bytes[offset++];
            mem_op->mem.scale = 1 << ((sib >> 6) & 0x03);
            mem_op->mem.index = (sib >> 3) & 0x07;
            mem_op->mem.base  = sib & 0x07;
            if (mem_op->mem.index == 4) mem_op->mem.index = -1;  // No index
            if (mem_op->mem.base == 5 && mod == 0) mem_op->mem.base = -1;  // disp32 only
        }

        // Special case: mod=00, rm=101 → [disp32] (no base register)
        if (mod == 0 && rm == 5) {
            mem_op->mem.base = -1;
        }

        // Read displacement
        if (mod == 1) {
            mem_op->mem.disp = (int8_t)bytes[offset++];  // Sign-extend!
            mem_op->mem.has_disp = true;
        } else if (mod == 2 || (mod == 0 && (rm == 5 || (rm == 4 && mem_op->mem.base == -1)))) {
            memcpy(&mem_op->mem.disp, bytes + offset, 4);  // Little-endian
            offset += 4;
            mem_op->mem.has_disp = true;
        }
    }

    return offset;
}
```

### 3.6 AT&T Formatter — objdump Output

Target format:
```
 8049000:	55                   	push   %ebp
 8049001:	89 e5                	mov    %esp,%ebp
 8049003:	83 ec 10             	sub    $0x10,%esp
 8049006:	8b 45 08             	mov    0x8(%ebp),%eax
 804900c:	7e 07                	jle    8049015 <func+0x15>
```

```c
// formatter.c

static const char *reg32_names[] = {"eax","ecx","edx","ebx","esp","ebp","esi","edi"};
static const char *reg16_names[] = {"ax","cx","dx","bx","sp","bp","si","di"};
static const char *reg8_names[]  = {"al","cl","dl","bl","ah","ch","dh","bh"};

void format_instruction(const instruction_t *inst, char *buf, size_t buflen) {
    int pos = 0;

    // Address
    pos += snprintf(buf + pos, buflen - pos, " %7x:\t", inst->address);

    // Raw bytes (left-justified in ~24 chars)
    int byte_pos = pos;
    for (int i = 0; i < inst->length; i++)
        pos += snprintf(buf + pos, buflen - pos, "%02x ", inst->bytes[i]);
    // Pad to column
    while (pos - byte_pos < 24)
        buf[pos++] = ' ';

    // Mnemonic
    pos += snprintf(buf + pos, buflen - pos, "\t%s", inst->mnemonic);

    // Operands (AT&T order: src, dst — REVERSED from Intel!)
    // ... format each operand
    // OP_REG:  "%%eax"
    // OP_IMM:  "$0x%x"
    // OP_MEM:  "0x%x(%%ebp)" or "(%%eax,%%ecx,4)" etc.
    // OP_REL:  "%x <symbol+offset>"
}

// Memory operand formatting:
void format_mem_operand(const operand_t *op, char *buf, size_t buflen) {
    int pos = 0;
    // displacement (if nonzero or only component)
    if (op->mem.has_disp || (op->mem.base == -1 && op->mem.index == -1))
        pos += snprintf(buf + pos, buflen - pos, "%s0x%x",
                        op->mem.disp < 0 ? "-" : "",
                        op->mem.disp < 0 ? -op->mem.disp : op->mem.disp);
    // (base, index, scale)
    if (op->mem.base >= 0 || op->mem.index >= 0) {
        pos += snprintf(buf + pos, buflen - pos, "(");
        if (op->mem.base >= 0)
            pos += snprintf(buf + pos, buflen - pos, "%%%s", reg32_names[op->mem.base]);
        if (op->mem.index >= 0)
            pos += snprintf(buf + pos, buflen - pos, ",%%%s,%d",
                            reg32_names[op->mem.index], op->mem.scale);
        pos += snprintf(buf + pos, buflen - pos, ")");
    }
}
```

### 3.7 Makefile

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -Iinclude
LDFLAGS =

SRC = src/main.c src/elf_parser.c src/decoder.c src/opcode_table.c src/formatter.c
OBJ = $(SRC:.c=.o)
BIN = disasm

$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Build 32-bit test binaries
tests/test_%: tests/test_%.c
	gcc -m32 -O0 -fno-stack-protector -fno-pie -no-pie -o $@ $<

# Compare against objdump
test: $(BIN) tests/test_arith tests/test_control tests/test_fib
	@for t in tests/test_*[^.c]; do \
		echo "Testing $$t..."; \
		objdump -d -M att $$t > /tmp/ref.txt; \
		./$(BIN) $$t > /tmp/mine.txt; \
		diff /tmp/ref.txt /tmp/mine.txt && echo "  PASS" || echo "  FAIL"; \
	done

clean:
	rm -f $(OBJ) $(BIN) tests/test_*[^.c]
```

### 3.8 Implementation Order (Build Incrementally!)

| Step | Instructions | Test With |
|------|-------------|-----------|
| 1 | `NOP`, `RET`, `LEAVE`, `CDQ` | Single-byte instructions first |
| 2 | `PUSH r32`, `POP r32`, `INC r32`, `DEC r32` | Opcode+rd encoding |
| 3 | `MOV r32, imm32` | B8+rd encoding |
| 4 | `MOV r/m32, r32`, `MOV r32, r/m32` | ModRM mod=11 (reg-reg) first |
| 5 | ModRM mod=01,10 (memory+disp) | `mov 0x8(%ebp),%eax` |
| 6 | ModRM mod=00 + SIB | Array access patterns |
| 7 | `ADD/SUB/CMP/AND/OR/XOR r/m, imm8` (opcode 83) | The `81/83` family with /0../7 |
| 8 | `JMP rel8`, `Jcc rel8` | Short jumps |
| 9 | `CALL rel32`, `JMP rel32`, `Jcc rel32` (0F 8x) | Two-byte opcodes |
| 10 | `LEA`, `IMUL`, `SHL/SHR/SAR` | Remaining instructions |
| 11 | `PUSH imm`, 8-bit operands, prefix 0x66 | Edge cases |

### 📚 Materials
- **Capstone Engine source** — `arch/X86/X86DisassemblerDecoder.c` — Clean reference implementation
- **GNU binutils** — `opcodes/i386-dis.c` — The real objdump decoder
- **Zydis disassembler** — Modern, very readable C code
- **"Practical Binary Analysis" by Dennis Andriesse** — Ch.8 (Custom Disassembly)

---

## Phase 3.5 — Stretch Goal: SETcc & MOVZX (Days 24–25)

> [!NOTE]
> Include these if your test programs use comparator-style returns like `return a > b;`. GCC emits `cmpl` → `setg %al` → `movzbl %al,%eax` for these.

| Instruction | Opcode | Encoding |
|-------------|--------|----------|
| `SETE r/m8` | `0F 94 /0` | Two-byte + ModRM (8-bit operand) |
| `SETNE r/m8` | `0F 95 /0` | |
| `SETG r/m8` | `0F 9F /0` | |
| `SETGE r/m8` | `0F 9D /0` | |
| `SETL r/m8` | `0F 9C /0` | |
| `SETLE r/m8` | `0F 9E /0` | |
| `SETA r/m8` | `0F 97 /0` | |
| `SETB r/m8` | `0F 92 /0` | |
| `MOVZX r32, r/m8` | `0F B6 /r` | ModRM, 8-bit source |
| `MOVZX r32, r/m16` | `0F B7 /r` | ModRM, 16-bit source |

**objdump renders MOVZX as `movzbl` (byte→long) or `movzwl` (word→long).**

---

## Phase 4 — Interactive Debugger (Days 26–38)

**Goal:** A fully interactive ptrace-based debugger with breakpoints, stepping, register/memory inspection, and disassembly.

### 4.1 Debugger Project Structure

```
debugger/
├── Makefile
├── include/
│   ├── debugger.h         # Core debugger state & API
│   ├── breakpoint.h       # Breakpoint management
│   ├── commands.h         # Command parser
│   └── disasm/            # Symlink or copy of Phase 3 headers
├── src/
│   ├── main.c             # CLI entry, fork+exec, command loop
│   ├── debugger.c         # ptrace wrappers, process control
│   ├── breakpoint.c       # INT3 insertion/removal
│   ├── commands.c         # Command parsing & dispatch
│   └── disasm/            # Reuse Phase 3 disassembler
└── Makefile
```

### 4.2 Core Debugger State

```c
// debugger.h

#include <sys/types.h>
#include <sys/user.h>       // struct user_regs_struct

#define MAX_BREAKPOINTS 64

typedef struct {
    uint32_t addr;          // Breakpoint address
    uint8_t  orig_byte;     // Original byte at addr (before INT3)
    bool     enabled;
    bool     hit;           // Currently stopped at this breakpoint?
    int      id;            // User-facing breakpoint number
} breakpoint_t;

typedef struct {
    pid_t    pid;                           // Tracee PID
    char     *binary_path;                  // Path to ELF binary
    bool     running;                       // Is tracee alive?
    breakpoint_t bps[MAX_BREAKPOINTS];      // Breakpoint list
    int      num_bps;
    // ELF info (reuse elf_parser from Phase 3):
    uint32_t text_addr;
    uint32_t text_size;
    // Symbol table for address→name lookups
    // ...
} debugger_t;
```

### 4.3 Process Launch: fork + exec + ptrace

```c
// main.c

#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

void debugger_run(debugger_t *dbg, const char *binary) {
    pid_t pid = fork();

    if (pid == 0) {
        // === CHILD (tracee) ===
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl(binary, binary, NULL);
        perror("execl failed");
        _exit(1);
    }

    // === PARENT (debugger) ===
    dbg->pid = pid;
    int status;
    waitpid(pid, &status, 0);   // Child stops on exec due to PTRACE_TRACEME

    if (WIFSTOPPED(status)) {
        printf("Process %d stopped at entry point\n", pid);
        dbg->running = true;
    }

    // Parse ELF for symbol table
    load_elf_symbols(dbg, binary);

    // Enter command loop
    command_loop(dbg);
}
```

### 4.4 Reading/Writing Tracee Memory & Registers

```c
// debugger.c

#include <sys/ptrace.h>
#include <sys/user.h>

// Read registers
void dbg_get_regs(debugger_t *dbg, struct user_regs_struct *regs) {
    ptrace(PTRACE_GETREGS, dbg->pid, NULL, regs);
}

// Get current EIP
uint32_t dbg_get_eip(debugger_t *dbg) {
    struct user_regs_struct regs;
    dbg_get_regs(dbg, &regs);
    return (uint32_t)regs.eip;
}

// Read a word from tracee memory
long dbg_peek(debugger_t *dbg, uint32_t addr) {
    return ptrace(PTRACE_PEEKTEXT, dbg->pid, (void *)(uintptr_t)addr, NULL);
}

// Write a word to tracee memory
void dbg_poke(debugger_t *dbg, uint32_t addr, long data) {
    ptrace(PTRACE_POKETEXT, dbg->pid, (void *)(uintptr_t)addr, (void *)data);
}

// Read N bytes from tracee (for disassembly)
void dbg_read_memory(debugger_t *dbg, uint32_t addr, uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len; i += sizeof(long)) {
        long word = dbg_peek(dbg, addr + i);
        size_t to_copy = (len - i < sizeof(long)) ? len - i : sizeof(long);
        memcpy(buf + i, &word, to_copy);
    }
}
```

### 4.5 Software Breakpoints (INT 3 = 0xCC)

```c
// breakpoint.c

int bp_set(debugger_t *dbg, uint32_t addr) {
    if (dbg->num_bps >= MAX_BREAKPOINTS) return -1;

    breakpoint_t *bp = &dbg->bps[dbg->num_bps];
    bp->addr = addr;
    bp->id = dbg->num_bps + 1;

    // Save original byte
    long word = dbg_peek(dbg, addr);
    bp->orig_byte = (uint8_t)(word & 0xFF);

    // Replace with INT 3 (0xCC)
    long modified = (word & ~0xFF) | 0xCC;
    dbg_poke(dbg, addr, modified);

    bp->enabled = true;
    dbg->num_bps++;
    printf("Breakpoint %d set at 0x%x\n", bp->id, addr);
    return bp->id;
}

void bp_restore(debugger_t *dbg, breakpoint_t *bp) {
    long word = dbg_peek(dbg, bp->addr);
    long restored = (word & ~0xFF) | bp->orig_byte;
    dbg_poke(dbg, bp->addr, restored);
}

// When breakpoint is hit: EIP has advanced past 0xCC, so decrement it
void bp_handle_hit(debugger_t *dbg) {
    uint32_t eip = dbg_get_eip(dbg);
    uint32_t bp_addr = eip - 1;  // EIP points AFTER the INT3

    for (int i = 0; i < dbg->num_bps; i++) {
        if (dbg->bps[i].addr == bp_addr && dbg->bps[i].enabled) {
            printf("Hit breakpoint %d at 0x%x\n", dbg->bps[i].id, bp_addr);

            // Restore original byte
            bp_restore(dbg, &dbg->bps[i]);

            // Rewind EIP to re-execute the original instruction
            struct user_regs_struct regs;
            dbg_get_regs(dbg, &regs);
            regs.eip = bp_addr;
            ptrace(PTRACE_SETREGS, dbg->pid, NULL, &regs);

            dbg->bps[i].hit = true;
            return;
        }
    }
}

// After single-stepping past a breakpoint, re-insert INT3
void bp_reinsert_after_step(debugger_t *dbg) {
    for (int i = 0; i < dbg->num_bps; i++) {
        if (dbg->bps[i].hit) {
            long word = dbg_peek(dbg, dbg->bps[i].addr);
            long patched = (word & ~0xFF) | 0xCC;
            dbg_poke(dbg, dbg->bps[i].addr, patched);
            dbg->bps[i].hit = false;
        }
    }
}
```

### 4.6 Process Control

```c
// debugger.c

void dbg_continue(debugger_t *dbg) {
    // If stopped at a breakpoint, single-step past it first
    for (int i = 0; i < dbg->num_bps; i++) {
        if (dbg->bps[i].hit) {
            ptrace(PTRACE_SINGLESTEP, dbg->pid, NULL, NULL);
            int status;
            waitpid(dbg->pid, &status, 0);
            bp_reinsert_after_step(dbg);
            break;
        }
    }

    ptrace(PTRACE_CONT, dbg->pid, NULL, NULL);
    int status;
    waitpid(dbg->pid, &status, 0);

    if (WIFSTOPPED(status)) {
        if (WSTOPSIG(status) == SIGTRAP) {
            bp_handle_hit(dbg);
        } else {
            printf("Stopped: signal %d\n", WSTOPSIG(status));
        }
    } else if (WIFEXITED(status)) {
        printf("Process exited with code %d\n", WEXITSTATUS(status));
        dbg->running = false;
    }
}

void dbg_single_step(debugger_t *dbg) {
    // Restore breakpoint byte if needed, step, then re-insert
    for (int i = 0; i < dbg->num_bps; i++) {
        if (dbg->bps[i].hit) {
            // Already restored in bp_handle_hit
        }
    }

    ptrace(PTRACE_SINGLESTEP, dbg->pid, NULL, NULL);
    int status;
    waitpid(dbg->pid, &status, 0);

    bp_reinsert_after_step(dbg);

    if (WIFSTOPPED(status)) {
        uint32_t eip = dbg_get_eip(dbg);
        printf("Stopped at 0x%x\n", eip);
        // Check if we landed on another breakpoint
        bp_handle_hit(dbg);
    }
}
```

### 4.7 Interactive Command Loop (with readline)

```c
// commands.c

#include <readline/readline.h>
#include <readline/history.h>

// Link with: -lreadline

void command_loop(debugger_t *dbg) {
    char *line;

    while ((line = readline("dbg> ")) != NULL) {
        if (strlen(line) > 0) add_history(line);

        char cmd[64], arg1[64], arg2[64];
        int nargs = sscanf(line, "%s %s %s", cmd, arg1, arg2);

        if (strcmp(cmd, "break") == 0 || strcmp(cmd, "b") == 0) {
            // break <address|symbol>
            uint32_t addr = resolve_address(dbg, arg1);  // hex or symbol
            bp_set(dbg, addr);

        } else if (strcmp(cmd, "continue") == 0 || strcmp(cmd, "c") == 0) {
            dbg_continue(dbg);

        } else if (strcmp(cmd, "step") == 0 || strcmp(cmd, "si") == 0) {
            dbg_single_step(dbg);

        } else if (strcmp(cmd, "regs") == 0 || strcmp(cmd, "info") == 0) {
            cmd_print_registers(dbg);

        } else if (strcmp(cmd, "x") == 0) {
            // x <addr> [count]
            uint32_t addr = strtoul(arg1, NULL, 0);
            int count = (nargs >= 3) ? atoi(arg2) : 16;
            cmd_examine_memory(dbg, addr, count);

        } else if (strcmp(cmd, "disas") == 0 || strcmp(cmd, "disassemble") == 0) {
            // disas [addr] [count]
            cmd_disassemble(dbg, nargs >= 2 ? arg1 : NULL,
                                 nargs >= 3 ? arg2 : NULL);

        } else if (strcmp(cmd, "bt") == 0 || strcmp(cmd, "backtrace") == 0) {
            cmd_backtrace(dbg);

        } else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "q") == 0) {
            if (dbg->running) {
                ptrace(PTRACE_KILL, dbg->pid, NULL, NULL);
            }
            free(line);
            break;

        } else {
            printf("Unknown command: %s\n", cmd);
            printf("Commands: break, continue, step, regs, x, disas, bt, quit\n");
        }

        free(line);
    }
}
```

### 4.8 Register Display

```c
void cmd_print_registers(debugger_t *dbg) {
    struct user_regs_struct regs;
    dbg_get_regs(dbg, &regs);

    printf("eax  0x%08lx    ebx  0x%08lx\n", regs.eax, regs.ebx);
    printf("ecx  0x%08lx    edx  0x%08lx\n", regs.ecx, regs.edx);
    printf("esi  0x%08lx    edi  0x%08lx\n", regs.esi, regs.edi);
    printf("ebp  0x%08lx    esp  0x%08lx\n", regs.ebp, regs.esp);
    printf("eip  0x%08lx    eflags 0x%08lx [", regs.eip, regs.eflags);

    // Decode EFLAGS
    if (regs.eflags & 0x0001) printf(" CF");
    if (regs.eflags & 0x0040) printf(" ZF");
    if (regs.eflags & 0x0080) printf(" SF");
    if (regs.eflags & 0x0800) printf(" OF");
    printf(" ]\n");
}
```

### 4.9 Disassemble at Current EIP (reuse Phase 3!)

```c
void cmd_disassemble(debugger_t *dbg, const char *addr_str, const char *count_str) {
    uint32_t addr;
    if (addr_str)
        addr = resolve_address(dbg, addr_str);
    else
        addr = dbg_get_eip(dbg);

    int count = count_str ? atoi(count_str) : 10;

    // Read memory from the running process
    uint8_t code[256];
    dbg_read_memory(dbg, addr, code, sizeof(code));

    // Disassemble using Phase 3 decoder
    int offset = 0;
    for (int i = 0; i < count && offset < (int)sizeof(code); i++) {
        instruction_t inst;
        int len = decode_instruction(code + offset, addr + offset, &inst);

        char buf[256];
        format_instruction(&inst, buf, sizeof(buf));

        // Mark current EIP with arrow
        uint32_t eip = dbg_get_eip(dbg);
        printf("%s %s\n", (addr + offset == eip) ? "=>" : "  ", buf);

        offset += len;
    }
}
```

### 4.10 Stack Backtrace (Walk EBP Chain)

```c
void cmd_backtrace(debugger_t *dbg) {
    struct user_regs_struct regs;
    dbg_get_regs(dbg, &regs);

    uint32_t eip = (uint32_t)regs.eip;
    uint32_t ebp = (uint32_t)regs.ebp;

    printf("#0  0x%08x", eip);
    const char *sym = lookup_symbol(dbg, eip);
    if (sym) printf(" in %s", sym);
    printf("\n");

    for (int frame = 1; frame < 20 && ebp != 0; frame++) {
        // Return address is at [EBP+4]
        uint32_t ret_addr = (uint32_t)dbg_peek(dbg, ebp + 4);
        // Previous EBP is at [EBP]
        uint32_t prev_ebp = (uint32_t)dbg_peek(dbg, ebp);

        printf("#%d  0x%08x", frame, ret_addr);
        sym = lookup_symbol(dbg, ret_addr);
        if (sym) printf(" in %s", sym);
        printf("\n");

        ebp = prev_ebp;
    }
}
```

### 4.11 Features — Implementation Order

| # | Feature | Difficulty | Days |
|---|---------|------------|------|
| 1 | fork+exec+PTRACE_TRACEME, waitpid | Easy | 1 |
| 2 | PTRACE_GETREGS → print registers | Easy | 0.5 |
| 3 | PTRACE_SINGLESTEP | Easy | 0.5 |
| 4 | PTRACE_PEEKTEXT → memory examine | Easy | 0.5 |
| 5 | readline command loop | Medium | 1 |
| 6 | Software breakpoints (INT 3) | **Hard** | 2 |
| 7 | Continue past breakpoints | **Hard** | 1.5 |
| 8 | `disas` using Phase 3 disassembler | Medium | 1 |
| 9 | Symbol resolution (break by name) | Medium | 1 |
| 10 | Backtrace (EBP chain walk) | Medium | 1 |
| 11 | Signal handling & clean exit | Medium | 1 |
| 12 | Edge cases & polish | Medium | 2 |

### 📚 Materials
- **`man 2 ptrace`** — Read the entire man page
- **"Writing a Linux Debugger" by Sy Brand** — [blog.tartanllama.xyz](https://blog.tartanllama.xyz/writing-a-linux-debugger-setup/) — 10-part series, builds this exact thing (C++)
- **"How Debuggers Work" by Eli Bendersky** — [eli.thegreenplace.net](https://eli.thegreenplace.net/2011/01/23/how-debuggers-work-part-1/) — 3-part series, essential
- **"Playing with ptrace" by Pradeep Padala** — Linux Journal, beginner-friendly
- **`<sys/user.h>`** — `struct user_regs_struct` definition
- **`<sys/wait.h>`** — `WIFSTOPPED`, `WSTOPSIG`, `WIFEXITED` macros
- **minidbg** on GitHub — Companion code to Sy Brand's blog

---

## Phase 5 — Integration, Edge Cases & Testing (Days 39–42)

### 5.1 Edge Cases to Handle

| Issue | Solution |
|-------|----------|
| Unknown opcode | Print `(bad)` or `.byte 0xNN` like objdump |
| `0x66` prefix changes operand size | Use 16-bit register names, `movw` instead of `movl` |
| Negative displacements | Print as `-0x4(%ebp)` not `0xfffffffc(%ebp)` |
| Multi-byte NOP (`0F 1F ...`) | Decode with ModRM, print as `nopl` / `nopw` |
| Jump targets outside .text | Print just the address, no symbol |
| Breakpoint on first byte of function | Handle EIP-1 = function start correctly |
| Signal delivery to tracee | Forward signals via `ptrace(PTRACE_CONT, pid, NULL, sig)` |

### 5.2 Test Programs

```c
// Compile all with:
// gcc -m32 -O0 -fno-stack-protector -fno-pie -no-pie -o test test.c

// test_basic.c — Data movement, stack frames
int basic(int a) { int x = a + 1; return x; }

// test_branches.c — All Jcc variants
int max(int a, int b) { return (a > b) ? a : b; }
int min(int a, int b) { return (a < b) ? a : b; }
int abs_val(int a) { return (a < 0) ? -a : a; }

// test_loop.c — Loops with CMP, JMP, INC
int sum(int n) { int s = 0; for (int i = 0; i < n; i++) s += i; return s; }

// test_div.c — Division (CDQ, IDIV)
int divide(int a, int b) { return a / b; }
int modulo(int a, int b) { return a % b; }

// test_logic.c — AND, OR, XOR, NOT, TEST
unsigned bitcount(unsigned n) {
    unsigned c = 0;
    while (n) { c += n & 1; n >>= 1; }
    return c;
}

// test_shift.c — SHL, SHR, SAR
int shift_test(int a) { return (a << 3) >> 1; }

// test_fib.c — Recursion: CALL, RET, stack frames
int fib(int n) {
    if (n <= 1) return n;
    return fib(n-1) + fib(n-2);
}
int main() { return fib(10); }

// test_array.c — SIB byte: scale*index+base
int array_sum(int *arr, int n) {
    int s = 0;
    for (int i = 0; i < n; i++) s += arr[i];  // Generates SIB for arr[i]
    return s;
}
```

### 5.3 Automated Testing Script

```bash
#!/bin/bash
# run_tests.sh — compare your output vs objdump

PASS=0; FAIL=0
for src in tests/test_*.c; do
    name=$(basename "$src" .c)
    gcc -m32 -O0 -fno-stack-protector -fno-pie -no-pie -o "/tmp/$name" "$src" 2>/dev/null
    if [ $? -ne 0 ]; then echo "SKIP $name (compile failed)"; continue; fi

    # Extract just .text disassembly, normalize whitespace
    objdump -d -M att "/tmp/$name" | sed -n '/<.*>:/,/^$/p' > "/tmp/ref_$name.txt"
    ./disasm "/tmp/$name" > "/tmp/my_$name.txt"

    if diff -q "/tmp/ref_$name.txt" "/tmp/my_$name.txt" > /dev/null 2>&1; then
        echo "PASS $name"; ((PASS++))
    else
        echo "FAIL $name"; ((FAIL++))
        diff --color "/tmp/ref_$name.txt" "/tmp/my_$name.txt" | head -20
    fi
done
echo "---"
echo "$PASS passed, $FAIL failed"
```

---

## Compilation & Dependencies

```bash
# You need 32-bit development libraries
sudo apt install gcc-multilib lib32readline-dev

# Build the disassembler
make -C disasm/

# Build the debugger (link readline)
gcc -Wall -g -Iinclude -o debugger src/*.c -lreadline

# Build a 32-bit test binary
gcc -m32 -O0 -fno-stack-protector -fno-pie -no-pie -o test32 test.c
```

---

## Complete Reading List

### Primary References (Must Read)

| Resource | What | Link |
|----------|------|------|
| Intel SDM Vol.1 Ch.3 | Register model, execution environment | [Intel SDM](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html) |
| Intel SDM Vol.2 Ch.2 | Instruction encoding format | Same |
| Intel SDM Vol.2 Appendix A | Opcode maps | Same |
| `man 5 elf` | ELF structs | `man 5 elf` |
| `man 2 ptrace` | Debugger system call | `man 2 ptrace` |
| ELF Spec (TIS) | Full ELF format | [elf.pdf](http://refspecs.linuxbase.org/elf/elf.pdf) |

### Books

| Book | Author | Chapters |
|------|--------|----------|
| CS:APP | Bryant & O'Hallaron | Ch.3 (x86), Ch.7 (ELF linking) |
| Practical Binary Analysis | Dennis Andriesse | Ch.1-2 (ELF), Ch.8 (disassembly) |
| Learning Linux Binary Analysis | Ryan O'Neill | Ch.1-2 (ELF), Ch.3-4 (ptrace) |
| Reverse Engineering for Beginners | Dennis Yurichev | Free, huge x86 reference |
| Programming from the Ground Up | Jonathan Bartlett | Free, x86+Linux intro |

### Blog Series

| Series | Author | URL |
|--------|--------|-----|
| Writing a Linux Debugger (10 parts) | Sy Brand | [tartanllama.xyz](https://blog.tartanllama.xyz/writing-a-linux-debugger-setup/) |
| How Debuggers Work (3 parts) | Eli Bendersky | [eli.thegreenplace.net](https://eli.thegreenplace.net/2011/01/23/how-debuggers-work-part-1/) |
| Playing with ptrace | Pradeep Padala | Linux Journal |

### Online References

| Resource | URL |
|----------|-----|
| x86 Opcode Reference | [ref.x86asm.net/coder32.html](http://ref.x86asm.net/coder32.html) |
| OSDev x86 Encoding | [wiki.osdev.org](https://wiki.osdev.org/X86-64_Instruction_Encoding) |
| sandpile.org Opcode Maps | [sandpile.org](https://sandpile.org/x86/opc_1.htm) |
| Felix Cloutier's x86 ref | [felixcloutier.com/x86](https://www.felixcloutier.com/x86/) |

### Source Code to Study

| Project | Why | Language |
|---------|-----|----------|
| Capstone Engine | Clean disassembler architecture | C |
| Zydis | Modern x86 decoder, very readable | C |
| GNU binutils (`opcodes/i386-dis.c`) | The real objdump | C |
| minidbg | Minimal debugger | C++ |

---

## Timeline Summary

```mermaid
flowchart LR
    A["Phase 0\nFoundations\nDays 1-3"] --> B["Phase 1\nELF Parser\nDays 4-7"]
    B --> C["Phase 2\nx86 Encoding\nDays 8-14"]
    C --> D["Phase 3\nDisassembler\nDays 15-24"]
    D --> D2["Phase 3.5\nSETcc/MOVZX\nDays 24-25"]
    D2 --> E["Phase 4\nDebugger\nDays 26-38"]
    E --> F["Phase 5\nPolish & Test\nDays 39-42"]
```

> [!TIP]
> **Phase 2 is the bottleneck.** Don't rush it. Spend time hand-decoding hex dumps with pencil and paper before writing any code. Once ModRM/SIB "clicks," everything else becomes mechanical.
>
> **Build incrementally in Phase 3.** Start with single-byte opcodes (NOP, RET), then register-encoded (PUSH r32), then ModRM reg-reg, then ModRM memory. Add one instruction at a time, test after each.
