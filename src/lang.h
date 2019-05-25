#ifndef LANG_H
#define LANG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

struct Script {
    uint16_t* rom;
    uint32_t* rom_ptr;
    
    uint16_t* stack;
    uint16_t* heap;
};

struct Script* script_load_from_file(const char* filename);
struct Script* script_load_from_memory(const char* memory);

#endif
