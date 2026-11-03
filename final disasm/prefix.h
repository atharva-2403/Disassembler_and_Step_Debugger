#ifndef PREFIX_H
#define PREFIX_H

typedef struct {
    int is_prefix;
    int group;//instructions belong to groups 1,2,3 and 4
    char name[16];
} prefix_info;

prefix_info get_prefix(unsigned char byte);//fetches prefix info for a given byte

#endif
