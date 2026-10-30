#include <stdio.h>
#include "prefix.h"
#include <string.h>

prefix_info get_prefix(unsigned char byte) {
        // By default, assume the byte is NOT a prefix                                          
        prefix_info info = {0, 0, ""};                                                          
                                                                                                
        switch (byte) {
            // Group 1: Lock and Repeat                                                         
            case 0xF0: info = (prefix_info){1, 1, "LOCK"}; break;
            case 0xF2: info = (prefix_info){1, 1, "REPNE"}; break;
            case 0xF3: info = (prefix_info){1, 1, "REP/REPE"}; break;

            // Group 2: Segment Overrides
            case 0x2E: info = (prefix_info){1, 2, "CS"}; break;
            case 0x36: info = (prefix_info){1, 2, "SS"}; break;
            case 0x3E: info = (prefix_info){1, 2, "DS"}; break;
            case 0x26: info = (prefix_info){1, 2, "ES"}; break;
            case 0x64: info = (prefix_info){1, 2, "FS"}; break;
            case 0x65: info = (prefix_info){1, 2, "GS"}; break;

            // Group 3: Operand-size Override                                                   
            case 0x66: info = (prefix_info){1, 3, "OP_SIZE"}; break;
                                                                                                
            // Group 4: Address-size Override                                                   
            case 0x67: info = (prefix_info){1, 4, "ADDR_SIZE"}; break;
        }
        return info;
    }
