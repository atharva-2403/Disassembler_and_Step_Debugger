
#include <stdio.h>
#include <string.h>
#include "decoder.h"        //reference to the function signature 
#include "instruction_table.h"      //reference to the lookup table we are using.

//converts the 3bit number into the identifier of a register
// for example, ppcode BB has lower three bits 011 which selects EBX
static x86_register_t get_register(uint8_t code)          //x86_register_t is the defined reg type in ins_framework
{
    static const x86_register_t registers[] = {
        REG_EAX, REG_ECX, REG_EDX, REG_EBX,
        REG_ESP, REG_EBP, REG_ESI, REG_EDI
    };
    return registers[code & 7]; // this is only to return the lower 3 bits, bitwise and opr.
}

//returns the actual reg name, and not its identifier or value.
static const char *register_name(x86_register_t reg) 
{
    static const char *names[] = {
        "none", "eax", "ecx", "edx", "ebx",
        "esp", "ebp", "esi", "edi"
    };
    return names[reg];
}

//reads the immediate byte, stored as 4 bytes
static uint32_t read_u32(const uint8_t *p)
{
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

//the main function, it gets the pointer to the bytes, total num of bytes, and start add of first byte
//int is the return type, bcus it only returns 0 or -1
int decode_instructions(const uint8_t *bytes, size_t size,
                        uint32_t start_address)
{
    size_t pos = 0;

    while (pos < size) {
        const instruction_rule_t *rule = NULL; //pointing to null value, as no ins identified yet
        uint8_t opcode = bytes[pos];

        for (unsigned int i = 0; i < instruction_table_count; i++) {
            if ((opcode & instruction_table[i].opcode_mask) ==
                instruction_table[i].opcode) {
                rule = &instruction_table[i];
                break;
            }
        }

        if (rule == NULL) {
            printf("0x%08X: db 0x%02X ; unsupported byte\n",
                   start_address + (uint32_t)pos, opcode);
            pos++;
            continue;
        }
        //
        if (size - pos < rule->fixed_length) {
            printf("0x%08X: incomplete instruction\n",
                   start_address + (uint32_t)pos);
            return -1;
        }

        printf("0x%08X: ", start_address + (uint32_t)pos);

        for (unsigned int j = 0; j < rule->fixed_length; j++)
            printf("%02X ", bytes[pos + j]);

        printf(" ");

        if (strcmp(rule->mnemonic, "mov") == 0) {
            x86_register_t reg = get_register(opcode);
            uint32_t value = read_u32(&bytes[pos + 1]);
            printf("mov %s, %u\n", register_name(reg), value);
        } else if (strcmp(rule->mnemonic, "add") == 0) {
            uint32_t value = read_u32(&bytes[pos + 1]);
            printf("add eax, %u\n", value);
        } else {
            printf("%s\n", rule->mnemonic);
        }

        pos += rule->fixed_length;
    }

    return 0;
}
