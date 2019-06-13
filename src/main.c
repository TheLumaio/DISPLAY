#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>

#include <raylib.h>
#include <raymath.h>
#include "raynames.h"

#include "display.h"
#include "lang.h"

typedef struct reflect_t {
    float d;
    Vector3 p;
    float y;
} reflect_t;

static void free_camera(Camera3D* c, float speed)
{
    // printf("FUCK");
    DisableCursor();
    
    static Vector3 camera_angle = {0.f};
    
    static Vector2 mouse_prev = {0.f};
    Vector2 mouse_delta = {0.f};
    Vector2 mouse_now = GetMousePosition();
    
    mouse_delta.x = mouse_now.x - mouse_prev.x;
    mouse_delta.y = mouse_now.y - mouse_prev.y;
    mouse_prev = mouse_now;
    
    camera_angle.x += mouse_delta.x*0.003;
    camera_angle.y += -mouse_delta.y*0.003;
    camera_angle.y = Clamp(camera_angle.y, -89.f*DEG2RAD, 89.f*DEG2RAD);
    // printf("%.2f\r", camera_angle.y);
    
    Vector3 front;
    front.x = cos(camera_angle.y) * cos(camera_angle.x);
    front.y = sin(camera_angle.y);
    front.z = cos(camera_angle.y) * sin(camera_angle.x);
    Vector3 cf = Vector3Normalize(front);
    
    if (IsKeyDown(KEY_W)) {
        float dx = cosf(camera_angle.x);
        float dy = sinf(camera_angle.x);
        c->position.x += dx*(speed*GetFrameTime());
        c->position.z += dy*(speed*GetFrameTime());
        // c->position = Vector3Add(c->position, Vector3Multiply(cf, speed*GetFrameTime()));
    }
    if (IsKeyDown(KEY_S)) {
        float dx = cosf(camera_angle.x);
        float dy = sinf(camera_angle.x);
        c->position.x -= dx*(speed*GetFrameTime());
        c->position.z -= dy*(speed*GetFrameTime());
        // c->position = Vector3Subtract(c->position, Vector3Multiply(cf, speed*GetFrameTime()));
    }
    if (IsKeyDown(KEY_A)) {
        c->position = Vector3Subtract(c->position, Vector3Multiply(Vector3Normalize(Vector3CrossProduct(cf, c->up)), speed*GetFrameTime()));
    }
    if (IsKeyDown(KEY_D)) {
        c->position = Vector3Add(c->position, Vector3Multiply(Vector3Normalize(Vector3CrossProduct(cf, c->up)), speed*GetFrameTime()));
    }
    
    c->target = Vector3Add(c->position, cf);
}

void draw_box(struct Display* display, int x, int y, int w, int h, uint8_t outer, uint8_t inner)
{
    display_print(display, "\xc9", x, y, outer);
    display_print(display, "\xbb", x+w, y, outer);
    display_print(display, "\xc8", x, y+h, outer);
    display_print(display, "\xbc", x+w, y+h, outer);
    
    for (int i = 0; i < w-1; i++) {
        display_print(display, "\xcd", x+1+i, y, outer);
        display_print(display, "\xcd", x+1+i, y+h, outer);
    }
    
    for (int i = 0; i < h-1; i++) {
        display_print(display, "\xba", x, y+1+i, outer);
        display_print(display, "\xba", x+w, y+1+i, outer);
    }
    
}

float smoothmin(float a, float b, float k)
{
    float h = fmax(k-fabs(a-b), 0) / k;
    return fmin(a, b) - h*h*h*k*1/6.0;
}

float sdOctahedron(Vector3 p, float s)
{
    Vector3 p2 = (Vector3){fabs(p.x), fabs(p.y), fabs(p.z)};
    return (p2.x+p2.y+p2.z-s)*0.57735027;
}

Vector2 DE(Vector3 p)
{
    Vector3 c = (Vector3){sin(get_time())*3, fabs(cos(get_time())*3), cos(get_time())*3};
    Vector3 c2 = (Vector3){cos(get_time())*3, fabs(cos(get_time())*10), sin(get_time())*3};
    if (p.y+10 < Vector3Length(Vector3Subtract(p, c))-1) {
        return (Vector2){p.y+10, 1};
    }
    else {
        float d1 = smoothmin(Vector3Length(Vector3Subtract(p, c2))-2, Vector3Length(Vector3Subtract(p, c))-1, 4);
        float d2 = fmin(d1, sdOctahedron(p, 3));
        if (d2 < d1) {
            float d3 = fmin(d2, Vector3Length(Vector3Subtract(p, (Vector3){0, 10, 0}))-0.4);
            if (d3 < d2) {
                return (Vector2){d3, 3};
            }
            return (Vector2){d2, 2};
        } else {
            return (Vector2){d2, 0};
        }
        
    }
}

reflect_t reflect( Vector3 ro, Vector3 rd, float mint, float maxt )
{
    float t = mint;
    for( t; t < maxt; )
    {
        // float h = map(ro + rd*t);
        Vector2 he = DE(Vector3Add(ro, Vector3Multiply(rd, t)));
        float h = he.x;
        if (he.y != 3) {
            if( h<0.001 )
                return (reflect_t){1, Vector3Add(ro, Vector3Multiply(rd, t)), he.y};
        }
        t += h;
    }
    return (reflect_t){0, Vector3Add(ro, Vector3Multiply(rd, t)), 0};
}

uint16_t get_cell(Vector3 p, float y)
{
    if (y == 0) {
        // display_putc(display, 0xb0+2-(intensity*2), display->width-1-_x, display->height-1-_y, c);
        return 0xb080;
    }
    else if (y == 1) {
        uint8_t c = 0x20;
        if (fabs(fmodf(floor(p.x)/5, 2)) == 0 || fabs(fmodf(floor(p.z)/5, 2)) == 0) {
            c = 0x30;
        }
        // display_putc(display, 0xb0+2-(intensity*2), display->width-1-_x, display->height-1-_y, c);
        return 0xb000 | c;
    }
    else if (y == 2) {
        // uint8_t c = 0x90;
        // if (reflect(p, normal, 1, 20)) {
        //     c |= 0xf8;
        // }
        
        // display_putc(display, 0xb0+2-(intensity*2), display->width-1-_x, display->height-1-_y, c);
        return 0xb090;
    }
    else if (y == 3) {
        return 0xb0b0;
    }
}

bool isch(char ch) {
    static const char* alphabet = " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-=!@#$%^&*()_+`~[]{}|\\,./<>?;':\"";
    for (int i = 0; i < 96; i++) {
        if (alphabet[i] == ch)
            return true;
    }
    return false;
}

int main(int argc, char** argv)
{
    init_window(640, 480, "DISPLAY");
    // set_target_fps(60);
    set_exit_key(KEY_F12);
    
    struct Script* s = script_load_from_memory("tst l: tst jmp l sys 0xb");
    
    display_load("font.png");
    struct Display* display = display_create(64, 48);
    
    // for (int i = 0; i < 7; i++) {
    //     display_putc(display, s->rom[i], 1+i, 5, s->rom[i]%256);
    // }
    
    float spin = 0;
    
    Camera camera = (Camera){(Vector3){0, 3, 3}, (Vector3){0, 0, 0}, (Vector3){0, 1, 0}, 90, 0};
    // set_camera_mode(camera, CAMERA_FIRST_PERSON);
    
    struct {
        int x;
        int y;
        float blink;
    } cursor;
    cursor.x = 4;
    cursor.y = 0;
    cursor.blink = 0;
    
    int current_line = 0;
    
    while (!window_should_close())
    {
        // display_putc(display, rand()%255, it, it, rand()%32);
        // spin+=get_frame_time()*5;
        // update_camera(&camera);
        // free_camera(&camera, 5);
        
        // float x = display->width/2;
        // float y = display->height/2;
        // float x2 = display->width/2;
        // float y2 = display->height/2;
        // while (x > 0 && y > 0 && x < display->width && y < display->height) {
        //     display_putc(display, rand()%256, (int)x, (int)y, abs(sin(get_time()*2)*0x0d));
        //     display_putc(display, rand()%256, (int)x2, (int)y2, abs(sin(get_time()*2)*0x0d));
        //     float dx = cos(spin);
        //     float dy = sin(spin);
        //     x += dx; x2 -= dx;
        //     y += dy; y2 -= dy;
        // }
        
        // Vector3 cam_pos = camera.position;//(Vector3){sin(get_time())*5, 2, cos(get_time())*5};
        // for (int i = 0; i < display->length; i++)
        // {
        //     int _x = i % display->width;
        //     int _y = i / display->width;
        //     // display_putc(display, 0x00, _x, _y, 0xaa);
            
        //     Vector2 q = (Vector2){(float)_x/(float)display->width, (float)_y/(float)display->height};
        //     Vector2 vpos = (Vector2){-1.0+2.0*q.x, -1.0+2.0*q.y};
        //     Vector3 vuv = (Vector3){0,1,0};
        //     Vector3 vrp = camera.target;
        //     Vector3 prp = cam_pos;
            
        //     Vector3 vpn = Vector3Normalize(Vector3Subtract(vrp,prp));
        //     Vector3 u = Vector3Normalize(Vector3CrossProduct(vuv,vpn));
        //     Vector3 v = Vector3CrossProduct(vpn,u);
        //     Vector3 vcv = Vector3Add(prp,vpn);
        //     Vector3 scr_coord = (Vector3){
        //         vcv.x+vpos.x*u.x*1.0+vpos.y*v.x*1.0,
        //         vcv.y+vpos.x*u.y*1.0+vpos.y*v.y*1.0,
        //         vcv.z+vpos.x*u.z*1.0+vpos.y*v.z*1.0
        //     };
        //     Vector3 scp = Vector3Normalize(Vector3Subtract(scr_coord,prp));
            
        //     const Vector3 e = (Vector3){0.02,0,0};
        //     const float maxd = 50.f;
        //     Vector2 d = (Vector2){0.02, 0};
        //     Vector3 c,p,N;
        //     float f = 1.f;
        //     Vector3 normal;
        //     for (int i = 0; i < 256; i++)
        //     {
        //         if ((fabs(d.x) < 0.001) || (f > maxd))
        //             break;
                
        //         f += d.x;
        //         p = Vector3Add(prp, Vector3Multiply(scp, f));
        //         d = DE(p);
        //     }
        //     if (f < maxd) {
        //         float h = 0.001;
        //         normal =  Vector3Normalize((Vector3){
        //             DE(Vector3Add(p, (Vector3){h, 0, 0})).x - DE(Vector3Subtract(p, (Vector3){h, 0, 0})).x,
        //             DE(Vector3Add(p, (Vector3){0, h, 0})).x - DE(Vector3Subtract(p, (Vector3){0, h, 0})).x,
        //             DE(Vector3Add(p, (Vector3){0, 0, h})).x - DE(Vector3Subtract(p, (Vector3){0, 0, h})).x,
        //         });
                
        //         // phong
        //         Vector3 light_dir = Vector3Normalize(Vector3Subtract((Vector3){0, 10, 0}, p));
        //         float intensity = Vector3DotProduct(normal, light_dir);
        //         intensity = fmax(0, intensity);
                
        //         uint16_t cell = get_cell(p, d.y);
                
        //         float s = reflect(p, light_dir, 1, 100).d;
        //         uint8_t c = 0xb0+2-(intensity*2);
        //         cell = (cell&0x00FF) | (c << 8);
                
        //         if (s>0 && p.y != 3) {
        //             cell = (cell&0x00FF) | (0xb200);
        //         }
        //         if (p.y != 1 && p.y != 3) {
        //             reflect_t r = reflect(p, normal, 1, 100);
        //             if (r.d>0) {
        //                 uint8_t ch = (cell&0xFF00)>>8;
        //                 uint8_t co = cell&0x00FF;
        //                 if (ch < 0xb2) ch+=0x01;
        //                 co = get_cell(r.p, r.y)&0x00FF;
        //                 cell = (ch<<8)|co;
        //             }
        //         }
                
        //         display_putc(display, cell>>8, display->width-1-_x, display->height-1-_y, cell);
                
        //     } else {
        //         display_putc(display, 0x00, display->width-1-_x, display->height-1-_y, 0xc0);
        //     }
        // }
        
        // display_print(display, format_text("FPS %d", get_fps()), 1, 1, 0x04);
        // display_print(display, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 1, 3, 0x0b);
        // display_print(display, "1234567890!@#$%^&*()[]\\{}|-=_+,./<>?;:'\"", 1, 4, 0x0b);
        
        // cursor.y = 3+current_line;
        
        // display_putc(display, 0xdb, cursor.x, cursor.y, 0x07);
        // char k = get_key_pressed();
        // if (isch(k)) {
        //     display_putc(display, k, cursor.x, cursor.y, 0x0b);
        //     cursor.x++;
        // }
        // if (is_key_pressed(KEY_BACKSPACE) && cursor.x > 4) {
        //     display_putc(display, 0x00, cursor.x, cursor.y, 0x00);
        //     cursor.x--;
        //     display_putc(display, 0x00, cursor.x, cursor.y, 0x00);  
        // }
        
        // display_print(display, "> ", 2, 3+current_line, 0x0b);
        
        
        
        begin_drawing();
        clear_background(BLACK);
        display_draw(display);
        end_drawing();
    }
    
    display_unload();
    display_free(display);
    
    close_window();
    
    return 0;
}
