#ifndef TEXT_SECTION_H
#define TEXT_SECTION_H

#include <stdint.h>

typedef struct{
	uint32_t text_section_offset;
	uint32_t size;
	uint32_t virtual_address;
}text_section_info;

#endif

