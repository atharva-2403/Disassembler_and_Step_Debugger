#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "text_section.h"
#include "prefix_metadata.h"
#include "decoded_instruction_standard.h"
#include "reference_table_standard.h"

#define DECODING_SUCCESS 2
#define DECODING_FAILURE 3

#define INS_LOOKUP_NULL 100
#define INS_LOOKUP_NONE 101

#define PIPE_READ_DESCRIPTOR 5

// Helper function to convert register enum to string
const char* reg_to_string_full(x86_reg_t reg) {
    switch(reg) {
        case AL: return "AL"; case BL: return "BL"; case CL: return "CL"; case DL: return "DL";
        case AH: return "AH"; case BH: return "BH"; case CH: return "CH"; case DH: return "DH";
        case AX: return "AX"; case BX: return "BX"; case CX: return "CX"; case DX: return "DX";
        case SP: return "SP"; case BP: return "BP"; case SI: return "SI"; case DI: return "DI";
        case CS: return "CS"; case DS: return "DS"; case SS: return "SS"; case ES: return "ES";
        case FS: return "FS"; case GS: return "GS";
        case EAX: return "EAX"; case ECX: return "ECX"; case EDX: return "EDX"; case EBX: return "EBX";
        case ESP: return "ESP"; case EBP: return "EBP"; case ESI: return "ESI"; case EDI: return "EDI";
        default: return "REG_NONE";
    }
}

// Helper function to convert segment enum to string
const char* seg_to_string(segment_t seg) {
    switch(seg) {
        case SEG_DEFAULT: return "SEG_DEFAULT";
        case SEG_ES: return "SEG_ES";
        case SEG_CS: return "SEG_CS";
        case SEG_SS: return "SEG_SS";
        case SEG_DS: return "SEG_DS";
        case SEG_FS: return "SEG_FS";
        case SEG_GS: return "SEG_GS";
        default: return "UNKNOWN_SEG";
    }
}

// Helper function to convert encoding type to string
const char* encoding_to_string(operand_encoding_t enc) {
    switch(enc) {
        case ENCODING_NONE: return "NONE";
        case ENCODING_RM: return "RM";
        case ENCODING_REG: return "REG";
        case ENCODING_OPCODE_REG: return "OPCODE_REG";
        case ENCODING_IMMEDIATE: return "IMMEDIATE";
        case ENCODING_RELATIVE: return "RELATIVE";
        case ENCODING_FAR_POINTER: return "FAR_POINTER";
        case ENCODING_FIXED_REG: return "FIXED_REG";
        case ENCODING_IMPLICIT: return "IMPLICIT";
        default: return "UNKNOWN";
    }
}

// Helper function to convert operand width to string
const char* width_to_string(operand_width_t w) {
    switch(w) {
        case WIDTH_NONE: return "NONE";
        case WIDTH_8: return "8-bit";
        case WIDTH_16: return "16-bit";
        case WIDTH_32: return "32-bit";
        case WIDTH_SIGN_EXT_8: return "SIGN_EXT_8";
        default: return "UNKNOWN";
    }
}

// Helper function to convert operand role to string
const char* role_to_string(operand_role_t role) {
    switch(role) {
        case OPERAND_SOURCE: return "SOURCE";
        case OPERAND_DESTINATION: return "DESTINATION";
        case OPERAND_SRC_DEST: return "SRC_DEST";
        default: return "UNKNOWN";
    }
}

// Comprehensive Python-style dumper for decoded_instruction_definition
void print_decoded_instruction_struct(const decoded_instruction_definition *instr) {
    printf("decoded_instruction_definition {\n");
    printf("  .address_in_elf         = 0x%08x,\n", instr->address_in_elf);
    printf("  .mnemonic               = \"%s\",\n", instr->mnemonic ? instr->mnemonic : "NULL");
    printf("  .prefix_count           = %d,\n", instr->prefix_count);
    
    if (instr->prefix_count > 0) {
        printf("  .prefixes               = [ ");
        for (int i = 0; i < instr->prefix_count; i++) {
            printf("\"%s\"%s", instr->prefixes[i].name ? instr->prefixes[i].name : "UNKNOWN", (i < instr->prefix_count - 1) ? ", " : " ");
        }
        printf("],\n");
    }

    printf("  .flags = {\n");
    printf("    .lock_enabled         = %s,\n", instr->lock_enabled ? "true" : "false");
    printf("    .repnz_enabled        = %s,\n", instr->repnz_enabled ? "true" : "false");
    printf("    .rep_enabled          = %s,\n", instr->rep_enabled ? "true" : "false");
    printf("    .seg_override_enabled = %s (%s),\n", instr->seg_override_enabled ? "true" : "false", seg_to_string(instr->segment_reg));
    printf("    .opsize_override      = %s,\n", instr->opsize_override_enabled ? "true" : "false");
    printf("    .addrsize_override    = %s,\n", instr->addrsize_override_enabled ? "true" : "false");
    printf("    .modrm_needed         = %s\n", instr->modrm_needed ? "true" : "false");
    printf("  },\n");

    printf("  .oprd_count             = %d,\n", instr->oprd_count);
    printf("  .operands               = [\n");
    
    for (int i = 0; i < instr->oprd_count; i++) {
        const operand_reference_t *op = &instr->operands[i];
        printf("    [%d] = {\n", i);
        printf("      .encoding_type      = %s,\n", encoding_to_string(op->encoding_type));
        printf("      .operand_width      = %s,\n", width_to_string(op->operand_width));
        printf("      .operand_role       = %s,\n", role_to_string(op->operand_role));
        printf("      .opsize_sensitive   = %s,\n", op->opsize_sensitive ? "true" : "false");
        printf("      .addrsize_sensitive = %s,\n", op->addrsize_sensitive ? "true" : "false");
        
        printf("      .operand_value      = ");
        switch (op->encoding_type) {
            case ENCODING_REG:
            case ENCODING_OPCODE_REG:
            case ENCODING_FIXED_REG:
                printf("{ .reg = %s }\n", reg_to_string_full(op->operand_value.reg));
                break;
            case ENCODING_IMMEDIATE:
                printf("{ .imm_val = 0x%x (%d) }\n", op->operand_value.imm_val, op->operand_value.imm_val);
                break;
            case ENCODING_RELATIVE:
                printf("{ .rel_val = %d }\n", op->operand_value.rel_val);
                break;
            case ENCODING_RM:
                printf("{ .reg = %s }\n", reg_to_string_full(op->operand_value.reg));
                break;
            default:
                printf("{ .raw = 0x%x }\n", op->operand_value.imm_val);
                break;
        }
        printf("    }%s\n", (i < instr->oprd_count - 1) ? "," : "");
    }
    printf("  ]\n");
    printf("}\n");
}

x86_reg_t resolveModrmReg(uint8_t reg_index, bool is_16bit) {
    switch(reg_index){
        case 0: return is_16bit ? AX : EAX;
        case 1: return is_16bit ? CX : ECX;
        case 2: return is_16bit ? DX : EDX;
        case 3: return is_16bit ? BX : EBX;
        case 4: return is_16bit ? SP : ESP;
        case 5: return is_16bit ? BP : EBP;
        case 6: return is_16bit ? SI : ESI;
        case 7: return is_16bit ? DI : EDI;
        default: return REG_NONE;
    }
}

const PrefixInfo* return_prefix(uint8_t byte) {
    if(prefix_table[byte].is_prefix) return &prefix_table[byte];
    else return NULL;
}

const int scanPrefixStream(uint8_t *begin, uint8_t *end, decoded_instruction_definition *instr) {
    *instr = (decoded_instruction_definition){0};
    const uint8_t *current = begin;
    instr->prefix_count = 0;

    while(current != end){
        uint8_t byte = *current;
        const PrefixInfo *prefix = return_prefix(byte);
        if(prefix == NULL) break;
        if(instr->prefix_count < 4){
            instr->prefixes[instr->prefix_count] = *prefix;
            switch(byte){
                case 0xF0: instr->lock_enabled = true; break;
                case 0xF2: instr->repnz_enabled = true; break;
                case 0xF3: instr->rep_enabled = true; break;
                case 0x26:
                case 0x2E:
                case 0x36:
                case 0x3E:
                case 0x64:
                case 0x65:
                    instr->seg_override_enabled = true;
                    instr->segment_reg = (segment_t)byte;
                    break;
                case 0x66: instr->opsize_override_enabled = true; break;
                case 0x67: instr->addrsize_override_enabled = true; break;
            }
            instr->prefix_count++;
        }
        current++;
    }
    return instr->prefix_count;
}

int scanOpcode(uint8_t *begin, uint8_t *end, decoded_instruction_definition *decoded_instr, uint32_t instruction_address) {
    uint8_t *current = begin;
    uint8_t byte = *current;
    int no_of_bytes = 1;
    printf("BYTE VALUE RECEIVED FOR DECODING : [%02x] : %2x\n", instruction_address, *current);
    
    opcode_table_type_t table_type;
    if(byte == 0x0F){
        no_of_bytes++;
        current += 1;
        byte = *current;
        if(byte == 0x38){
            table_type = TABLE_TERTIARY_38;
            no_of_bytes++;
            current += 1;
            byte = *current;
        }
        else if(byte == 0x3A){
            table_type = TABLE_TERTIARY_3A;
            no_of_bytes++;
            current += 1;
            byte = *current;
        }
        else {
            table_type = TABLE_SECONDARY;
        }
    }
    else {
        table_type = TABLE_PRIMARY;
    }
    
    byte = *current;
    opcode_table_entry_t *ref_opcode_info = get_opcode_entry(table_type, byte);
    
    if(ref_opcode_info == NULL) return INS_LOOKUP_NULL;
    if(ref_opcode_info->type == ENTRY_TYPE_NONE) return INS_LOOKUP_NONE;

    if(ref_opcode_info->type == ENTRY_TYPE_SINGLE || ref_opcode_info->type == ENTRY_TYPE_EXTENSION_GROUP){
        
        instruction_reference_t *current_ref_opcode_content = NULL;
        uint8_t modrm_mod = 0, modrm_reg = 0, modrm_rm = 0;
        
        switch(ref_opcode_info->type){
            case ENTRY_TYPE_SINGLE:
                current_ref_opcode_content = &ref_opcode_info->content.single_instruction;
                bool modrm_req = current_ref_opcode_content->modrm_required;
                if(modrm_req){
                    no_of_bytes++;
                    current += 1;
                    uint8_t modrm_byte = *current;
                    modrm_mod = (modrm_byte >> 6) & 0x03;
                    modrm_reg = (modrm_byte >> 3) & 0x07;
                    modrm_rm = modrm_byte & 0x07;   
                }
                break;

            case ENTRY_TYPE_EXTENSION_GROUP:
                no_of_bytes++;
                current += 1;
                uint8_t modrm_byte = *current;
                modrm_mod = (modrm_byte >> 6) & 0x03;
                modrm_reg = (modrm_byte >> 3) & 0x07;
                modrm_rm = modrm_byte & 0x07;
                current_ref_opcode_content = &ref_opcode_info->content.extension_bucket[modrm_reg];
                break;
        }

        decoded_instr->mnemonic = current_ref_opcode_content->mnemonic;
        decoded_instr->modrm_needed = current_ref_opcode_content->modrm_required;
        decoded_instr->oprd_count = current_ref_opcode_content->operand_count;
        decoded_instr->address_in_elf = instruction_address;

        for(int i = 0; i < current_ref_opcode_content->operand_count; i++){
            operand_reference_t *current_operand = &current_ref_opcode_content->operands[i];
            operand_encoding_t encoding = current_operand->encoding_type;
            bool opsz_sensitive = current_operand->opsize_sensitive;
            bool width_toggle = opsz_sensitive && decoded_instr->opsize_override_enabled;
            
            switch(encoding){
                case ENCODING_REG:
                    decoded_instr->operands[i].encoding_type = ENCODING_REG;
                    decoded_instr->operands[i].operand_width = width_toggle ? WIDTH_16 : WIDTH_32;
                    decoded_instr->operands[i].operand_value.reg = resolveModrmReg(modrm_reg, width_toggle);
                    decoded_instr->operands[i].operand_role = current_operand->operand_role;
                    decoded_instr->operands[i].opsize_sensitive = current_operand->opsize_sensitive;
                    decoded_instr->operands[i].addrsize_sensitive = current_operand->addrsize_sensitive;
                    break;

                case ENCODING_RM:
                    decoded_instr->operands[i].encoding_type = ENCODING_REG;
                    decoded_instr->operands[i].operand_width = width_toggle ? WIDTH_16 : WIDTH_32;
                    decoded_instr->operands[i].operand_value.reg = resolveModrmReg(modrm_rm, width_toggle);
                    decoded_instr->operands[i].operand_role = current_operand->operand_role;
                    decoded_instr->operands[i].opsize_sensitive = current_operand->opsize_sensitive;
                    decoded_instr->operands[i].addrsize_sensitive = current_operand->addrsize_sensitive;            
                    break;

                case ENCODING_OPCODE_REG: {
                    uint8_t reg_index = byte & 0x07;
                    decoded_instr->operands[i].encoding_type = ENCODING_OPCODE_REG;
                    decoded_instr->operands[i].operand_width = width_toggle ? WIDTH_16 : WIDTH_32;
                    decoded_instr->operands[i].operand_value.reg = resolveModrmReg(reg_index, width_toggle);
                    decoded_instr->operands[i].operand_role = current_operand->operand_role;
                    decoded_instr->operands[i].opsize_sensitive = current_operand->opsize_sensitive;
                    decoded_instr->operands[i].addrsize_sensitive = current_operand->addrsize_sensitive;
                    break;
                }
                
                case ENCODING_IMMEDIATE: {
                    operand_width_t width = current_operand->operand_width;
                    decoded_instr->operands[i].encoding_type = ENCODING_IMMEDIATE;
                    decoded_instr->operands[i].operand_role = current_operand->operand_role;
                    decoded_instr->operands[i].operand_width = width;
                    decoded_instr->operands[i].opsize_sensitive = current_operand->opsize_sensitive;
                    decoded_instr->operands[i].addrsize_sensitive = current_operand->addrsize_sensitive;
                    
                    switch(width){
                        case WIDTH_8:
                            current += 1;
                            decoded_instr->operands[i].operand_value.imm_val = (uint32_t)(*current);
                            break;
                        case WIDTH_SIGN_EXT_8:
                            current += 1;
                            decoded_instr->operands[i].operand_value.imm_val = (uint32_t)(int32_t)(*(int8_t *)current);
                            break;
                        case WIDTH_16: {
                            uint16_t val = 0;
                            val |= (uint16_t)(*++current);
                            val |= (uint16_t)(*++current) << 8;
                            decoded_instr->operands[i].operand_value.imm_val = (uint32_t)val;
                            break;
                        }
                        case WIDTH_32: {
                            uint32_t val = 0;
                            val |= (uint32_t)(*++current);
                            val |= (uint32_t)(*++current) << 8;
                            val |= (uint32_t)(*++current) << 16;
                            val |= (uint32_t)(*++current) << 24;
                            decoded_instr->operands[i].operand_value.imm_val = val;
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                }
                
                default:
                    break;
            }
        }
    }
    return (current - begin) + 1;
}

void decoder(uint8_t *buffer, int size, uint32_t text_section_address) {
    uint8_t *end = buffer + size;
    decoded_instruction_definition instructions[1024] = {0};
    int instruction_count = 0;
    
    printf("ALERT : Initialising the Opcode tables.\n");
    init_opcode_tables();
    printf("ALERT : Opcode tables initialised.\n");
    printf("\nDECODING STARTED.\n");
    
    uint8_t *current = buffer;
    uint32_t text_sect_addr = text_section_address; 
    
    while(current < end){
        int no_of_prefixes = scanPrefixStream(current, end, &instructions[instruction_count]);
        current += no_of_prefixes;
        text_sect_addr += no_of_prefixes;
        if(current >= end) break;

        int offset = scanOpcode(current, end, &instructions[instruction_count], text_sect_addr);
        current += offset;
        text_sect_addr += offset;

        if(offset == INS_LOOKUP_NULL || offset == INS_LOOKUP_NONE) {
            printf("[INSTRUCTION UNSUPPORTED]\n");
            printf("Terminating\n");
            break;
        }
        else {
            printf("[INSTRUCTION DECODING SUCCESS]\n");
            instruction_count++;
        }
    }
    printf("INSTRUCTIONS DECODED : %d\n", instruction_count);
    printf("POINT ALEPH\n");

    if (instruction_count > 0) {
        printf("Last Instruction Processed : \n");
        print_decoded_instruction_struct(&instructions[instruction_count - 1]);
    }
        
    printf("TERMINATING PROGRAM\n");
}

int begin_decoding(char *filename) {
    printf("\nSTAGE 3 >> DECODING PHASE BEGINS\n");
    printf("STAGE 3 >> Creating struct to receive information.\n");
    text_section_info info;
    
    printf("STAGE 3 >> Piping information from STAGE 2\n"); 
    ssize_t bytes_rd = read(PIPE_READ_DESCRIPTOR, &info, sizeof(info));
    if(bytes_rd == -1){
        perror("Information Piping failure");
        return DECODING_FAILURE;
    }
    printf("STAGE 3 >> Closing Pipe.\n");
    close(PIPE_READ_DESCRIPTOR);

    printf("STAGE 3 >> Opening the ELF.\n");    
    int fd_elf = open(filename, O_RDONLY);
    if(fd_elf == -1){
        perror("STAGE 3 >> Failure to open ELF.");
        close(fd_elf);
        return DECODING_FAILURE;
    }
    
    uint32_t text_section_address = info.text_section_offset;
    off_t status = lseek(fd_elf, info.text_section_offset, SEEK_SET);
    if(status == (off_t)-1){
        perror("STAGE 3 >> Lseek failed");
        close(fd_elf);
        return DECODING_FAILURE;
    }
    
    printf("STAGE 3 >> Creating a storage buffer\n");
    unsigned char *text_section_buffer = malloc(info.size);
    
    printf("STAGE 3 >> Buffer created, Beginning Read.\n");
    ssize_t bytes_read = read(fd_elf, text_section_buffer, info.size);
    if(bytes_read == -1){
        perror("STAGE 3 >> Failure to read ELF");
        free(text_section_buffer);
        close(fd_elf);
        return DECODING_FAILURE;
    }
    
    printf("STAGE 3 >> Read Complete, Closing file.\n");
    close(fd_elf);
    
    printf("STAGE 3 >> Invoking the decoder.\n");
    decoder(text_section_buffer, info.size, text_section_address);
    
    free(text_section_buffer);
    return DECODING_SUCCESS;
}

int main(int argc, char *argv[]) {
    if(argc != 2) return 1;
    char *filename = argv[1];
    begin_decoding(filename);
    return 0;
}