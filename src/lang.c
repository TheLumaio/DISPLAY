#include "lang.h"

struct _label {
    char lname[128];
    uint16_t location;
};

#define push_op(op) \
    if (cur_rom == max_rom) { \
        rom = realloc(rom, sizeof(uint16_t)*(max_rom*2)); \
        max_rom *= 2; \
    } \
    rom[cur_rom++] = op;

#define push_label(name, loc) \
    if (cur_label == max_label) { \
        labels = realloc(labels, sizeof(struct _label)*(max_label*2)); \
        max_label *= 2; \
    } \
    strcpy(labels[cur_label].lname, name); \
    labels[cur_label++].location = loc;

static uint16_t* lexer(const char* source)
{
    
    uint16_t* rom = malloc(sizeof(uint16_t)*1024);
    uint16_t max_rom = 1024;
    uint16_t cur_rom = 0;
    
    struct _label* labels = malloc(sizeof(struct _label)*128);
    uint16_t max_label = 128;
    uint16_t cur_label = 0;
    
    char* str = strdup(source);
    char* token = strtok(str, " ");
    
    while (token)
    {
        uint16_t op = 0x0000;
        
        if (token[strlen(token)-1] == ':') {
            push_op(0x1);
            push_op(cur_rom-2);
            token[strlen(token)-1] = '\0';
            push_label(token, cur_rom-2);
        }
        if (strcmp(token, "tst") == 0) {
            push_op(0x2);
        }
        if (strcmp(token, "jmp") == 0) {
            push_op(0x3);
            token = strtok(NULL, " ");
            for (int i = 0; i < cur_label; i++) {
                if (strcmp(labels[i].lname, token) == 0) {
                    push_op(labels[i].location);
                    break;
                }
            }
        }
        if (strcmp(token, "sys") == 0) {
            push_op(0x4);
            token = strtok(NULL, " ");
            if (token) {
                push_op((uint16_t)strtol(token, NULL, 0));
            }
        }
        
        token = strtok(NULL, " ");
    }
    
    free(token);
    free(str);
    
    return rom;
}

struct Script* script_load_from_memory(const char* memory)
{
    struct Script* script = malloc(sizeof(struct Script));
    script->rom = lexer(memory);
    script->rom_ptr = 0;
}
