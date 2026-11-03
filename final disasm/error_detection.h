#include <stdio.h>
#include <elf.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>

//defining constants
#define MAX_WARNINGS                32
#define MAX_PHNUM_VALUE             32
#define MAX_PHNUM_VALUE_FATAL       128
#define MAX_SHNUM_VALUE_FATAL       4096
#define MAX_SHNUM_VALUE             256

//a buffer for all the warnings. 
static int warning_codes[MAX_WARNINGS]; 
static int warning_count = 0;


//500 series Alerts (ERROR Alerts)
#define FILE_READ_ERROR                     500
#define FILE_FORMAT_NOT_RECG                501
#define FILE_BITNESS_INVALID                502
#define FILE_ENDIANNESS_INVALID             503
#define FILE_VERSION_INVALID                504
#define FILE_OSABI_INVALID                  505
#define FILE_CORRUPT                        506
#define FILE_TYPE_UNSUPPORTED               507
#define MACHINE_TYPE_UNSUPPORTED            508
#define FILE_SEEK_ERROR                     509
//570 ONWARDS : CORRUPTION TYPES
#define CORRUPT_ELF_HEADER                  570
#define CORRUPT_PROGRAM_HEADER              571
#define CORRUPT_PHT_OFFSET_BEYOND_EOF       572
#define CORRUPT_PHENTSIZE_INCONSISTENT      573
#define CORRUPT_FATAL_PHNUM                 574
#define CORRUPT_ENTRY_POINT                 575
#define CORRUPT_EHSIZE_VAL_INCONSISTENT     576
#define CORRUPT_PHOFF_VAL_INCONSISTENT      577
#define CORRUPT_SHENTSIZE_INCONSISTENT      578
#define CORRUPT_FATAL_SHNUM                 579
#define CORRUPT_PHTABLE_BEYOND_EOF          580
#define CORRUPT_SHOFF_VAL_INCONSISTENT      581
#define CORRUPT_SHTABLE_BEYOND_EOF          582
#define CORRUPT_TABLES_OVERLAP              583
#define CORRUPT_PHDR_ENTRY                  584
#define CORRUPT_PHDR_ENTRY_INCONGRUENT      585
#define CORRUPT_SHSTRNDX_VAL_INCONSISTENT   586
#define CORRUPT_STRTAB                      587
#define CORRUPT_SECTION_HEADER              588
#define CORRUPT_SHT_NULL_DIRTY              589
#define CORRUPT_SHT_PROGBITS_DIRTY          590
#define CORRUPT_PHDR_PT_NULL_DIRTY          591
#define CORRUPT_PHDR_PT_LOAD_DIRTY          592
#define CORRUPT_PHDR_PT_DYNAMIC_DIRTY       593
#define CORRUPT_PHDR_PT_INTERP_DIRTY        594
#define CORRUPT_PHDR_PT_NOTE_DIRTY          595
#define CORRUPT_PHDR_PT_SHLIB_DIRTY         596
#define CORRUPT_PHDR_PT_PHDR_DIRTY          597
//600 series Alerts (GENERAL Alerts)
#define FILE_SCAN_SUCCESS                   600
#define WARNING_LOG_FAILURE                 601
#define WARNING_LOG_SUCCESS                 602
#define PHDR_ENTRY_READ_SUCCESS             603
//700 series Alerts (WARNING Alerts)
#define FILE_PADDING_ANOMALY                700
#define ABNORMAL_E_FLAGS                    701
#define ABNORMAL_PHNUMS                     702
#define ABNORMAL_SHNUMS                     703
#define ABNORMAL_PHNUM_VAL_ZERO             704
#define ABNORMAL_SHNUM_VAL_ZERO             705
#define ABNORMAL_PT_NULL_NON_ZERO           706
#define ABNORMAL_UNKNOWN_PHDR_SEGMENT       707
#define ABNORMAL_SHSTRTAB_UNDEF             708
#define ABNORMAL_SHSTRTAB_SHN_XINDEX        709
#define ABNORMAL_PHDR_PT_LOAD               710
#define ABNORMAL_PHDR_PT_INTERP             711
#define ABNORMAL_PHDR_PT_NOTE               712
#define ABNORMAL_PT_SHLIB_USED              713
#define ABNORMAL_PHDR_PT_PHDR               714
#define ABNORMAL_PHDR_PT_GNU_STACK          715



void printMessage(int error_code){
    switch(error_code){
        case FILE_READ_ERROR:
            printf("ERROR %d : File read failure\n",error_code);
            break;
        case FILE_SEEK_ERROR:
            printf("ERROR %d : File seek failure\n",error_code);
            break;
        case FILE_FORMAT_NOT_RECG:
            printf("ERROR %d : File format not recognised. [Magic Bytes Mismatch]\n",error_code);
            break;
        case FILE_SCAN_SUCCESS:
            printf("ALERT %d : File scanned successfully. No errors detected.\n",error_code);
            break;
        case FILE_BITNESS_INVALID:
            printf("ERROR %d : File Bitness not supported.\n",error_code);
            break;
        case FILE_ENDIANNESS_INVALID:
            printf("ERROR %d : File Endianess not supported.\n",error_code);
            break;
        case FILE_VERSION_INVALID:
            printf("ERROR %d : File Version not supported.\n",error_code);
            break;
        case FILE_OSABI_INVALID:
            printf("ERROR %d : File ABI not supported.\n",error_code);
            break;
        case MACHINE_TYPE_UNSUPPORTED:
            printf("ERROR %d : Machine type not supported.\n",error_code);
            break;

        case FILE_PADDING_ANOMALY:
            printf("WARNING %d : e_ident padding bytes are non-zero. Potential tampering / File corruption of the ELF Header (e_ident_array).\n", error_code);
            break;
        case ABNORMAL_E_FLAGS:
            printf("WARNING %d : E_flags field is non-zero. Potential tampering / File corruption of the ELF Header.\n", error_code);
            break;
        case ABNORMAL_PHNUMS:
            printf("WARNING %d : Abnormal value of phnum detected. Potential tampering / File corruption of the ELF Header. \n",error_code);
            break;
        case ABNORMAL_PHNUM_VAL_ZERO:
            printf("WARNING %d : ELF header claims number of program header as zero.\n",error_code);
            break;
        case ABNORMAL_SHNUM_VAL_ZERO:
            printf("WARNING %d : ELF header claims number of section header as zero.\n",error_code);
            break;
        case ABNORMAL_PT_NULL_NON_ZERO:
            printf("WARNING %d : Program header entry PT_NULL has non zero values. Possiblt tampering / File corruption detected.\n",error_code);
            break;



        case FILE_CORRUPT:
            printf("ERROR %d : File Corruption Detected.\n",error_code);
            break;
        case CORRUPT_ELF_HEADER:
            printf("ERROR %d : CORRUPTION DETECTED (Elf header corrupted).\n",error_code);
            break;
        case CORRUPT_PROGRAM_HEADER:
            printf("ERROR %d : CORRUPTION DETECTED (Program header corrupted.)\n",error_code);
            break;
        case CORRUPT_PHT_OFFSET_BEYOND_EOF:
            printf("Error %d : CORRUPTION DETECTED (Elf header possibly corrupt/tampered. Program Header table beyond EOF.)\n",error_code);
            break;
        case CORRUPT_PHENTSIZE_INCONSISTENT:
            printf("ERROR %d : CORRUPTION DETECTED (Elf header possibly corrupt/tampered. Ehdr claims inconsistent Program header size.)\n",error_code);
            break;
        case CORRUPT_SHENTSIZE_INCONSISTENT:
            printf("ERROR %d : CORRUPTION DETECTED (Elf header possibly corrupt/tampered. Ehdr claims inconsistent Section header size.)\n",error_code);
            break;
        case CORRUPT_FATAL_PHNUM:
            printf("Error %d : CORRUPTION DETECTED (Elf header possibly corrupt/tampered. Abnormally large number of program headers.)\n",error_code);
            break;
        case CORRUPT_ENTRY_POINT:
            printf("Error %d : CORRUPTION DETECTED (Entry Point Corrupt.)\n",error_code);
            break;
        case CORRUPT_EHSIZE_VAL_INCONSISTENT:
            printf("Error %d : CORRUPTION DETECTED (Elf header possibly corrupt/tampered. Value of ehsize inconsistent.)\n",error_code);
            break;
        case CORRUPT_PHOFF_VAL_INCONSISTENT:
            printf("Error %d : CORRUPTION DETECTED (Elf header possibly corrupt/tampered. Value of program header offset inconsistent.)\n",error_code);
            break;
        case CORRUPT_SHOFF_VAL_INCONSISTENT:
            printf("Error %d : CORRUPTION DETECTED (Elf header possibly corrupt/tampered. Value of section header offset inconsistent.)\n",error_code);
            break;
        case CORRUPT_PHTABLE_BEYOND_EOF:
            printf("Error %d : CORRUPTION DETECTED (Elf header possibly corrupt/tampered. Program Header table beyond EOF.)\n",error_code);
            break;
        case CORRUPT_SHTABLE_BEYOND_EOF:
            printf("Error %d : CORRUPTION DETECTED (Elf header possibly corrupt/tampered. Section Header table beyond EOF.)\n",error_code);
            break;
        case CORRUPT_TABLES_OVERLAP:
            printf("Error %d : CORRUPTION DETECTED (Elf header possibly corrupt/tampered. Table paraments in Ehdr indivate destructive overlapping.)\n",error_code);
            break;
        
        case CORRUPT_PHDR_ENTRY:
            printf("Error %d : CORRUPTION DETECTED (Program header table possible corrupt/tampered. Program header entry inconsistent.)\n",error_code);
            break;
        case CORRUPT_PHDR_PT_NULL_DIRTY:
            printf("Error %d : CORRUPTION DETECTED (Phdr Entry PT_NULL inconsistent. Possible corruption/tampering detected.)\n",error_code);
            break;    
        case CORRUPT_PHDR_PT_LOAD_DIRTY:
            printf("Error %d : CORRUPTION DETECTED (Phdr Entry PT_LOAD inconsistent. Possible corruption/tampering detected.)\n",error_code);
            break;
        case CORRUPT_PHDR_PT_NOTE_DIRTY:
            printf("Error %d : CORRUPTION DETECTED (Phdr Entry PT_NOTE inconsistent. Possible corruption/tampering detected.)\n",error_code);
            break;
        case CORRUPT_PHDR_PT_DYNAMIC_DIRTY:
            printf("Error %d : CORRUPTION DETECTED (Phdr Entry PT_DYNAMIC inconsistent. Possible corruption/tampering detected.)\n",error_code);
            break;
        case CORRUPT_PHDR_PT_INTERP_DIRTY:
            printf("Error %d : CORRUPTION DETECTED (Phdr Entry PT_INTERP inconsistent. Possible corruption/tampering detected.)\n",error_code);
            break;
        case CORRUPT_PHDR_PT_PHDR_DIRTY:
            printf("Error %d : CORRUPTION DETECTED (Phdr Entry PT_PHDR inconsistent. Possible corruption/tampering detected.)\n",error_code);
            break;
        case CORRUPT_PHDR_PT_SHLIB_DIRTY:
            printf("Error %d : CORRUPTION DETECTED (Phdr Entry PT_SHLIB inconsistent. Possible corruption/tampering detected.)\n",error_code);
            break;
        
        default:
            printf("%d : INVALID ALERT CODE\n",error_code);
            break;
    }
}
int log_warning(int code){
    if(warning_count<MAX_WARNINGS){
        warning_codes[warning_count] = code; 
        warning_count++;
        return WARNING_LOG_SUCCESS;
    }
    else{
        return WARNING_LOG_FAILURE;
    }
}

void printfWarnings(){
    for(int i=0;i<warning_count;i++){
        printMessage(warning_codes[i]);
    }
}

int validate_program_header_entry(bool *entry_validity, Elf32_Addr entrypoint, Elf32_Phdr *entry,off_t file_size){

    switch(entry->p_type){
        
        case PT_NULL:
            //in this program header entry, there's nothing. what's concerning is when there's actually something in there. 
            //if the values are non zero, then its either corrupt, or maybe there's some tampering. 
            if(entry->p_offset != 0 || entry->p_vaddr != 0  || entry->p_paddr != 0 || 
                   entry->p_filesz != 0 || entry->p_memsz != 0 || entry->p_flags != 0 || 
                   (entry->p_align != 0 && entry->p_align != 1)) {
                    log_warning(ABNORMAL_PT_NULL_NON_ZERO);
            }
            break;
        
        case PT_LOAD:
            if(entry->p_filesz > entry->p_memsz){
                return CORRUPT_PHDR_PT_LOAD_DIRTY;
            }
            if (entry->p_filesz > 0) {
                unsigned long long seg_end = (unsigned long long)entry->p_offset + entry->p_filesz;
                if (seg_end > (unsigned long long)file_size || seg_end < entry->p_offset) {
                return CORRUPT_PHDR_PT_LOAD_DIRTY;
                }
            }


            //checking is the alignment is a power of two.
            if(entry->p_align > 1){
                if((entry->p_align & (entry->p_align - 1)) != 0){
                    return CORRUPT_PHDR_PT_LOAD_DIRTY;
                }
                if ((entry->p_vaddr % entry->p_align) != (entry->p_offset % entry->p_align)) {
                    return CORRUPT_PHDR_PT_LOAD_DIRTY;
                }
            }
            //checking the flags now. 
            if(entry->p_flags == 0){
                return CORRUPT_PHDR_PT_LOAD_DIRTY;
            }
            if ((entry->p_flags & (PF_R | PF_W | PF_X)) == 0) {
                return CORRUPT_PHDR_PT_LOAD_DIRTY;
            }
            if((entry->p_flags & PF_W) && !(entry->p_flags & PF_R)){
                return CORRUPT_PHDR_PT_LOAD_DIRTY;
            }
            if ((entry->p_flags & PF_X) && !(entry->p_flags & PF_R)) {
                log_warning(ABNORMAL_PHDR_PT_LOAD); 
            }
            if ((entry->p_flags & PF_W) && (entry->p_flags & PF_X)) {
                log_warning(ABNORMAL_PHDR_PT_LOAD);
            }

            if(entry->p_type == PT_LOAD){
                Elf32_Addr start_addr = entry->p_vaddr;
                Elf32_Addr end_addr = entry->p_vaddr + entry->p_memsz;
                if(entrypoint >= start_addr && entrypoint < end_addr){
                    if(entry->p_flags & PF_X) *entry_validity = true;
                }
            }
            break;

        case PT_DYNAMIC: {            
            //size consistency check
            if(entry->p_filesz>entry->p_memsz){
                return CORRUPT_PHDR_PT_DYNAMIC_DIRTY;
            }
            //checking the file boundary conditions and the overflow conditions/
            if (entry->p_filesz > 0) {
                unsigned long long seg_end = (unsigned long long)entry->p_offset + entry->p_filesz;
                if (seg_end > (unsigned long long)file_size || seg_end < entry->p_offset) {
                    return CORRUPT_PHDR_PT_DYNAMIC_DIRTY;
                }
            }
            if (entry->p_flags == 0 || !(entry->p_flags & PF_R)) {
                return CORRUPT_PHDR_PT_DYNAMIC_DIRTY; // Must allow reading
            }
            if (entry->p_flags & PF_X) {
                return CORRUPT_PHDR_PT_DYNAMIC_DIRTY;
            }

            // 4. Optional: Structure alignment check (should be aligned to at least 4 bytes for 32-bit)
            if (entry->p_align > 1 && (entry->p_align & (entry->p_align - 1)) != 0) {
                return CORRUPT_PHDR_PT_DYNAMIC_DIRTY;
            }
            break;
        }

        case PT_INTERP: {
            if (entry->p_filesz == 0 || entry->p_filesz > 256) {
                return CORRUPT_PHDR_PT_INTERP_DIRTY;
            }
            unsigned long long seg_end = (unsigned long long)entry->p_offset + entry->p_filesz;
            if (seg_end > (unsigned long long)file_size || seg_end < entry->p_offset) {
                return CORRUPT_PHDR_PT_INTERP_DIRTY;
            }
            if (entry->p_flags == 0 || !(entry->p_flags & PF_R)) {
                return CORRUPT_PHDR_PT_INTERP_DIRTY;
            }
            if (entry->p_flags & PF_X) {
                log_warning(ABNORMAL_PHDR_PT_INTERP);
            }
            break;
        }

        case PT_NOTE: {
            if (entry->p_filesz > 0) {
                unsigned long long seg_end = (unsigned long long)entry->p_offset + entry->p_filesz;
                if (seg_end > (unsigned long long)file_size || seg_end < entry->p_offset) {
                    return CORRUPT_PHDR_PT_NOTE_DIRTY;
                }
            }
            if (entry->p_filesz > entry->p_memsz && entry->p_memsz != 0) {
                return CORRUPT_PHDR_PT_NOTE_DIRTY;
            }
            if (entry->p_flags & PF_X) {
                log_warning(ABNORMAL_PHDR_PT_NOTE); 
            }

            break;
        }

        case PT_SHLIB: {
            log_warning(ABNORMAL_PT_SHLIB_USED);
            if (entry->p_filesz > 0) {
                unsigned long long seg_end = (unsigned long long)entry->p_offset + entry->p_filesz;
                if (seg_end > (unsigned long long)file_size || seg_end < entry->p_offset) {
                    return CORRUPT_PHDR_PT_SHLIB_DIRTY;
                }
            }
            break;
        }

        case PT_PHDR: {
            if (entry->p_filesz > entry->p_memsz) {
                return CORRUPT_PHDR_PT_PHDR_DIRTY;
            }
            if (entry->p_filesz > 0) {
                unsigned long long seg_end = (unsigned long long)entry->p_offset + entry->p_filesz;
                if (seg_end > (unsigned long long)file_size || seg_end < entry->p_offset) {
                    return CORRUPT_PHDR_PT_PHDR_DIRTY;
                }
            }
            if (entry->p_flags == 0 || !(entry->p_flags & PF_R)) {
                return CORRUPT_PHDR_PT_PHDR_DIRTY; 
            }
            if (entry->p_flags & PF_W) {
                log_warning(ABNORMAL_PHDR_PT_PHDR);
            }

            break;
        }
        case PT_GNU_STACK: {
            if (entry->p_filesz != 0) {
                log_warning(ABNORMAL_PHDR_PT_GNU_STACK);
            }
            if (entry->p_flags & PF_X) {
                log_warning(ABNORMAL_PHDR_PT_GNU_STACK);
            }
            if (!(entry->p_flags & PF_R) || !(entry->p_flags & PF_W)) {
                log_warning(ABNORMAL_PHDR_PT_GNU_STACK);
            }

            break;
        }
        case PT_LOPROC ... PT_HIPROC: {
            if (entry->p_filesz > 0) {
                unsigned long long seg_end = (unsigned long long)entry->p_offset + entry->p_filesz;
                if (seg_end > (unsigned long long)file_size || seg_end < entry->p_offset) {
                    return CORRUPT_PHDR_ENTRY;
                }
            }
            break;
        }
        default:
            //log_warning(ABNORMAL_UNKNOWN_PHDR_SEGMENT);
            if (entry->p_filesz > 0) {
                unsigned long long seg_end = (unsigned long long)entry->p_offset + entry->p_filesz;
                if (seg_end > (unsigned long long)file_size || seg_end < entry->p_offset) {
                    return CORRUPT_PHDR_ENTRY;
                }
            }
            break;
    }
    return PHDR_ENTRY_READ_SUCCESS;
}

int validate_section_header(Elf32_Shdr *shdr,int idx, Elf32_Shdr *strtab_ptr, off_t file_size,Elf32_Ehdr *ehdr){

    switch(shdr->sh_type){
        case SHT_NULL: {
            if(idx == 0) {
                if(shdr->sh_type != SHT_NULL)              return CORRUPT_SHT_NULL_DIRTY;
                if(shdr->sh_name != 0)                     return CORRUPT_SHT_NULL_DIRTY;
                if(shdr->sh_flags != 0)                    return CORRUPT_SHT_NULL_DIRTY;
                if(shdr->sh_addr != 0)                     return CORRUPT_SHT_NULL_DIRTY;
                if(shdr->sh_offset != 0)                   return CORRUPT_SHT_NULL_DIRTY;
                if(shdr->sh_entsize != 0)                  return CORRUPT_SHT_NULL_DIRTY;
                if(shdr->sh_type != SHT_NULL || shdr->sh_name != 0 || shdr->sh_addr != 0 || shdr->sh_entsize != 0) {
                    return CORRUPT_SHT_NULL_DIRTY;
                }
                if (shdr->sh_addralign != 0 && shdr->sh_addralign != 1) {
                    return CORRUPT_SHT_NULL_DIRTY;
                }
                if(shdr->sh_size != 0 && ehdr->e_shnum != 0) {
                    return CORRUPT_SHT_NULL_DIRTY;
                }
                if(shdr->sh_link != 0 && ehdr->e_shstrndx != 0xffff) {
                    return CORRUPT_SHT_NULL_DIRTY;
                }
                if (shdr->sh_info != 0 && ehdr->e_phnum != 0xffff) {
                    return CORRUPT_SHT_NULL_DIRTY;
                }
            }
            else{
                if (shdr->sh_name != 0 || shdr->sh_flags != 0 || shdr->sh_addr != 0 || 
                    shdr->sh_offset != 0 || shdr->sh_size != 0 || shdr->sh_link != 0 || 
                    shdr->sh_info != 0 || shdr->sh_addralign != 0 || shdr->sh_entsize != 0) {
                    return CORRUPT_SHT_NULL_DIRTY;
                }
            }
            break;
        }
    }
}

int detect_errors(char *file){

    int fd = open(file,O_RDWR);

    //scanning the file size
    off_t file_size = lseek(fd,0,SEEK_END);
    if(file_size == (off_t)-1){
        close(fd);
        return FILE_READ_ERROR;
    }
    lseek(fd,0,SEEK_SET);
    
    //creating a buffer to read the ELF Header that contains the magic bytes.

    Elf32_Ehdr elf32_header;

    ssize_t bytes_read = read(fd,&elf32_header,sizeof(elf32_header));
    if(bytes_read == -1) {
        close(fd);
        return FILE_READ_ERROR;

    }

    //validating the magic bytes now. 

    unsigned char *file_info = elf32_header.e_ident;
    if( !(file_info[EI_MAG0] == ELFMAG0 &&
        file_info[EI_MAG1] == ELFMAG1 &&
        file_info[EI_MAG2] == ELFMAG2 &&
        file_info[EI_MAG3] == ELFMAG3)){
            close(fd);
            return FILE_FORMAT_NOT_RECG;
    }
    //magic bytes scan complete.
    
    //validating the 32 bit architecture check now. 

    if(file_info[EI_CLASS] != ELFCLASS32){
        if(file_info[EI_CLASS] == ELFCLASS64){
            close(fd);
            return FILE_BITNESS_INVALID;
        }
        else{
            close(fd);
            return FILE_CORRUPT;
        }
    }

    if(file_info[EI_DATA] != ELFDATA2LSB){
        if(file_info[EI_DATA] == ELFDATA2MSB){
            close(fd);
            return FILE_ENDIANNESS_INVALID;
        }
        else{
            close(fd);
            return FILE_CORRUPT;
        }
    }

    if(file_info[EI_VERSION] != EV_CURRENT){
        if(file_info[EI_VERSION] == EV_NONE){
            close(fd);
            return FILE_VERSION_INVALID;
        }
        else{
            close(fd);
            return FILE_CORRUPT;
        }
    }
    if(file_info[EI_OSABI] != ELFOSABI_SYSV && file_info[EI_OSABI] != ELFOSABI_LINUX){
        if(file_info[EI_OSABI] == ELFOSABI_HPUX || 
                file_info[EI_OSABI] == ELFOSABI_NETBSD || 
                file_info[EI_OSABI] == ELFOSABI_SOLARIS || 
                file_info[EI_OSABI] == ELFOSABI_IRIX || 
                file_info[EI_OSABI] == ELFOSABI_FREEBSD || 
                file_info[EI_OSABI] == ELFOSABI_TRU64 || 
                file_info[EI_OSABI] == ELFOSABI_ARM || 
                file_info[EI_OSABI] == ELFOSABI_STANDALONE ||
                file_info[EI_OSABI] == ELFOSABI_AIX ||
                file_info[EI_OSABI] == ELFOSABI_MODESTO ||
                file_info[EI_OSABI] == ELFOSABI_OPENBSD ||
                file_info[EI_OSABI] == ELFOSABI_ARM_AEABI) {
                    close(fd);
                    return FILE_OSABI_INVALID;
        }
        else{
            close(fd);
            return FILE_CORRUPT;
        }
    }

    //NOW CHECKING FOR THE PADDING ANOMALY
    bool is_zero = true;
    for(int i=EI_PAD;i<EI_NIDENT;i++){
        if(file_info[i] != 0){
            is_zero = false;
            break;
        }
    }
    if(!is_zero) log_warning(FILE_PADDING_ANOMALY);


    uint16_t file_type = elf32_header.e_type;
    if(file_type < 1 || file_type >4){
        if(file_type == 0){
            close(fd);
            return FILE_TYPE_UNSUPPORTED;
        }
        else{
            close(fd);
            return FILE_CORRUPT;
        }
    }

    uint16_t machine_type = elf32_header.e_machine;
    if(machine_type != EM_386){
        close(fd);
        return MACHINE_TYPE_UNSUPPORTED;
    }

    //This is not a redundant test. This is for corruption checking.
    uint32_t version_info = elf32_header.e_version;
    if(version_info != EV_CURRENT){
        if(version_info == EV_NONE){
            close(fd);
            return FILE_VERSION_INVALID;
        }
        else{
            close(fd);
            return FILE_CORRUPT;
        }
    }

    if(elf32_header.e_phoff > 0 && elf32_header.e_phoff < sizeof(Elf32_Ehdr)){
        close(fd);
        return CORRUPT_PHOFF_VAL_INCONSISTENT;
    }
    if((off_t)elf32_header.e_phoff >=file_size){
        close(fd);
        return CORRUPT_PHT_OFFSET_BEYOND_EOF;
    }

    if(elf32_header.e_shoff > 0 && elf32_header.e_shoff < sizeof(Elf32_Ehdr)){
        close(fd);
        return CORRUPT_SHOFF_VAL_INCONSISTENT;
    }

    //checking the e_flags memberr
    if(elf32_header.e_flags != 0){
        log_warning(ABNORMAL_E_FLAGS);
    }
    
    //chking the ehsize member for value corruption
    if(elf32_header.e_ehsize != sizeof(Elf32_Ehdr)){
        close(fd);
        return CORRUPT_EHSIZE_VAL_INCONSISTENT;
    }
    
    //checking the program header size consistency now. 
    if(elf32_header.e_phentsize != sizeof(Elf32_Phdr)){
        close(fd);
        return CORRUPT_PHENTSIZE_INCONSISTENT;
    }
    
    if(elf32_header.e_phnum == 0) log_warning(ABNORMAL_PHNUM_VAL_ZERO);
    if(elf32_header.e_phnum > MAX_PHNUM_VALUE){
        log_warning(ABNORMAL_PHNUMS);
    }

    //checking the phnum value validity now.
    if(elf32_header.e_phnum > MAX_PHNUM_VALUE_FATAL){
        close(fd);
        return CORRUPT_FATAL_PHNUM;
    }

    unsigned long long pht_total_size = (unsigned long long)elf32_header.e_phnum * elf32_header.e_phentsize;;
    if((off_t)(elf32_header.e_phoff + pht_total_size) > file_size) {
        close(fd);
        return CORRUPT_PHTABLE_BEYOND_EOF; //the table is extnding past end of file
    }

    //========================================================================================================================
    //========================================================================================================================
    //========================================================================================================================


    if(elf32_header.e_shnum == 0) log_warning(ABNORMAL_SHNUM_VAL_ZERO);
    if(elf32_header.e_shnum > MAX_SHNUM_VALUE){
        log_warning(ABNORMAL_SHNUMS);
    }
    if(elf32_header.e_shnum > MAX_SHNUM_VALUE_FATAL){
        close(fd);
        return CORRUPT_FATAL_SHNUM;
    }

    if(elf32_header.e_shentsize != sizeof(Elf32_Shdr)){
        close(fd);
        return CORRUPT_SHENTSIZE_INCONSISTENT;
    }

    unsigned long long sht_total_size = (unsigned long long)elf32_header.e_shnum * elf32_header.e_shentsize;;
    if((off_t)(elf32_header.e_shoff + sht_total_size) > file_size) {
        close(fd);
        return CORRUPT_SHTABLE_BEYOND_EOF; //the table is extnding past end of file
    }

    //checking for intersection between the values. 

    if (elf32_header.e_phnum > 0 && elf32_header.e_shnum > 0 && elf32_header.e_shoff > 0 && elf32_header.e_phoff > 0) {
        unsigned long long pht_start = elf32_header.e_phoff;
        unsigned long long pht_end = pht_start + pht_total_size;
        
        unsigned long long sht_start = elf32_header.e_shoff;
        unsigned long long sht_end = sht_start + sht_total_size;

        if (pht_start<sht_end && pht_end> sht_start) {
            close(fd);
            return CORRUPT_TABLES_OVERLAP; 
        }
    }

    //now we will analyse the shstrndx member of the elf_header.
    uint16_t str_tab_offset = elf32_header.e_shstrndx;
    Elf32_Shdr *str_tab_ptr;
    //first off, the index of the string table section should be less than the shnum val. 
    if(elf32_header.e_shnum > 0){
        if(elf32_header.e_shstrndx == SHN_UNDEF){ 
            log_warning(ABNORMAL_SHSTRTAB_UNDEF);
        }
        else if(elf32_header.e_shstrndx == SHN_XINDEX){
            log_warning(ABNORMAL_SHSTRTAB_SHN_XINDEX);
        }
        else if(elf32_header.e_shstrndx>=elf32_header.e_shnum){
            close(fd);
            return CORRUPT_SHSTRNDX_VAL_INCONSISTENT;
        }
        else{
            off_t shstr_entry_offset = (off_t)elf32_header.e_shoff + ((off_t)elf32_header.e_shstrndx * elf32_header.e_shentsize);
            if (lseek(fd, shstr_entry_offset, SEEK_SET) == (off_t)-1) {
            close(fd);
            return FILE_SEEK_ERROR;
            }
            Elf32_Shdr strtab_hdr;
            ssize_t shdr_bytes_read = read(fd, &strtab_hdr, sizeof(Elf32_Shdr));
            str_tab_ptr = &strtab_hdr;
            if(shdr_bytes_read != (ssize_t)sizeof(Elf32_Shdr)) {
                close(fd);
                return CORRUPT_SHTABLE_BEYOND_EOF;
            }
            if(strtab_hdr.sh_type != SHT_STRTAB){
                close(fd);
                return CORRUPT_STRTAB;
            }
            unsigned long long strtab_end = (unsigned long long)strtab_hdr.sh_offset + strtab_hdr.sh_size;
            if (strtab_end > (unsigned long long)file_size || strtab_end < strtab_hdr.sh_offset) {
                close(fd);
                return CORRUPT_SHTABLE_BEYOND_EOF;
            }
        }
    
    }

    //========================================================================================================================
    //=================================================[ ELF HEADER CHECK COMPLETE ]==========================================
    //============================================(NO THIS IS NOT AN AI GENERATED CODE)=======================================
    //=============================================(YES I WROTE IT WITH MY OWN HANDS)=========================================
    //========================================================================================================================

    bool entry_validity = false;
    if(elf32_header.e_phnum > MAX_PHNUM_VALUE){
        log_warning(ABNORMAL_PHNUMS);
    }
    off_t ph_table_offset = (off_t)elf32_header.e_phoff;
    if(lseek(fd,ph_table_offset,SEEK_SET) == (off_t)-1){
        close(fd);
        return FILE_SEEK_ERROR;
    }
    if(elf32_header.e_phnum != 0){
        size_t total_phdr_size = elf32_header.e_phnum * sizeof(Elf32_Phdr);
        Elf32_Phdr *ph_table = malloc(total_phdr_size);
        ssize_t bytes_read = read(fd,ph_table,total_phdr_size);
        if(bytes_read != (ssize_t)total_phdr_size){
            free(ph_table);
            close(fd);
            printf("Point Aleph");
            return CORRUPT_PROGRAM_HEADER;
        }
        for(int i=0;i<elf32_header.e_phnum;i++){
            Elf32_Phdr *phdr = &ph_table[i];
            int status = validate_program_header_entry(&entry_validity,elf32_header.e_entry,phdr,file_size);
            if(status != PHDR_ENTRY_READ_SUCCESS){
                close(fd);
                free(ph_table);
                return status;
            }
        }
    }

    off_t sh_table_offset = (off_t)elf32_header.e_shoff;
    if(lseek(fd,sh_table_offset,SEEK_SET) == (off_t)-1){
        close(fd);
        return FILE_SEEK_ERROR;
    }
    if(elf32_header.e_shnum != 0){
        size_t total_shdr_size = elf32_header.e_shnum * sizeof(Elf32_Shdr);
        Elf32_Shdr *sh_table = malloc(total_shdr_size);
        ssize_t bytes_read = read(fd,sh_table,total_shdr_size);
        if(bytes_read != (ssize_t)total_shdr_size){
            free(sh_table);
            close(fd);
            return CORRUPT_SECTION_HEADER;
        }
        for(int i=0;i<elf32_header.e_shnum;i++){
            Elf32_Shdr *shdr = &sh_table[i];
            //the code goes in this now. 

            //I'LL RELEASE THIS IN THE NEXT UPDATE
        }
    }
    close(fd);
    return FILE_SCAN_SUCCESS;
}

