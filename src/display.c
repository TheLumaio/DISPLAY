#include "display.h"

void display_load(const char* font_file)
{
    font_texture = load_texture(font_file);
    framebuffer = load_render_texture(get_screen_width(), get_screen_height());
    
    memset(changes, 0, sizeof(uint32_t)*MAX_CHANGES);
    
    loaded = true;
}

void display_unload()
{
    unload_texture(font_texture);
    unload_render_texture(framebuffer);
    loaded = false;
}

struct Display* display_create(int width, int height)
{
    if (!loaded) return NULL;
    struct Display* display = malloc(sizeof(struct Display));
    display->width = width;
    display->height = height;
    display->length = width*height;
    display->need_update = false;
    
    display->buffer = malloc(sizeof(uint16_t)*display->length);
    memset(display->buffer, 0x0000, sizeof(uint16_t)*display->length);
    
    for (int i = 0; i < display->length; i++) {
        if (changes_made == MAX_CHANGES) {
            display_draw(display);
        }
        changes[changes_made++] = i;
    }
    
    return display;
}

void display_free(struct Display* display)
{
    // free(display->buffer);
    free(display);
}

void display_print(struct Display* display, const char* msg, int x, int y, uint8_t flags)
{
    for (int i = 0; i < strlen(msg); i++)
    {
        uint16_t fgbg = flags << 8;
        display->buffer[(x+i)*display->height+y] = fgbg | DISPLAYTOFONT(msg[i]);
        if (changes_made == MAX_CHANGES) {
            display_draw(display);
        }
        changes[changes_made++] = (x+i)*display->height+y;
    }
    display->need_update = true;
}

void display_putc(struct Display* display, uint8_t c, int x, int y, uint8_t flags)
{
    uint16_t fgbg = flags << 8;
    
    if (display->buffer[x*display->height+y] == (fgbg | DISPLAYTOFONT(c))) return; // +200-300 fps lmao
    display->buffer[x*display->height+y] = fgbg | DISPLAYTOFONT(c);
    if (changes_made == MAX_CHANGES) {
        display_draw(display);
    }
    changes[changes_made++] = x*display->height+y;
}

void display_force_change(struct Display* display, int index)
{
    if (changes_made == MAX_CHANGES) {
        display_draw(display);
    }
    changes[changes_made++] = index;
}

void display_draw(struct Display* display)
{
    if (changes_made > 0) {
        begin_texture_mode(framebuffer);
        
        for (int i = 0; i < changes_made; i++)
        {
            int j = changes[i];
            
            int _x = j / display->height;
            int _y = j % display->height;
            
            int ch = display->buffer[j]&0x00ff;
            int fg = (display->buffer[j]&0x0f00)>>8;
            
            int _fx = ch % 16;
            int _fy = ch / 16;
            
            draw_rectangle(_x*10, _y*10, 10, 10, display_colors[(display->buffer[j]&0xf000)>>12]);
            draw_texture_rec(font_texture, (Rectangle){_fx*10, _fy*10, 10, 10}, (Vector2){_x*10, _y*10}, display_colors[fg]);
        }
        changes_made = 0;
        
        end_texture_mode();
        display->need_update = false;
    }
    
    draw_texture_rec(framebuffer.texture, (Rectangle){0, 0, framebuffer.texture.width, -framebuffer.texture.height}, (Vector2){0, 0}, WHITE);
}

uint8_t display_tofont(uint8_t ch)
{
    if (ch >= 'A' && ch <= 'Z') {
        return 0x0041+(ch-'A');
    }
    if (ch >= 'a' && ch <= 'z') {
        return 0x0061+(ch-'a');
    }
    if (ch == ' ') return 0x0000;
    if (ch == '[') return 0x005b;
    if (ch == '\\') return 0x005c;
    if (ch == ']') return 0x005d;
    else return ch;
}
