#ifndef DISPLAY_H
#define DISPLAY_H

#ifndef DISPLAYTOFONT
#define DISPLAYTOFONT display_tofont
#endif

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <raylib.h>
#include "raynames.h"

#define MAX_CHANGES 1024

struct Display {
    int width;
    int height;
    int length;
    uint16_t* buffer;
    bool need_update;
};

static Color display_colors[16] = {
    (Color){24, 24, 24, 255},     // 0x0
    (Color){40, 40, 40, 255},     // 0x1
    (Color){56, 56, 56, 255},     // 0x2
    (Color){88, 88, 88, 255},     // 0x3
    (Color){184, 184, 184, 255},  // 0x4
    (Color){216, 216, 216, 255},  // 0x5
    (Color){232, 232, 232, 255},  // 0x6
    (Color){248, 248, 248, 255},  // 0x7
    (Color){171, 70, 66, 255},    // 0x8
    (Color){220, 150, 86, 255},   // 0x9
    (Color){247, 202, 136, 255},  // 0xa
    (Color){161, 181, 108, 255},  // 0xb
    (Color){134, 193, 185, 255},  // 0xc
    (Color){124, 175, 194, 255},  // 0xd
    (Color){186, 139, 175, 255},  // 0xe
    (Color){161, 105, 70, 255}    // 0xf
};

static Texture font_texture;
static bool loaded = false;

static RenderTexture2D framebuffer;

static uint32_t changes[MAX_CHANGES];
static uint32_t changes_made = 0;

void display_load(const char* font_file);
void display_unload();
struct Display* display_create(int width, int height);
void display_free(struct Display* display);
void display_print(struct Display* display, const char* msg, int x, int y, uint8_t flags);
void display_putc(struct Display* display, uint8_t c, int x, int y, uint8_t flags);
void display_force_change(struct Display* display, int index);
void display_draw(struct Display* display);
uint8_t display_tofont(uint8_t ch);

#endif
