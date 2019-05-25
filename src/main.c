#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <raylib.h>
#include <raymath.h>
#include "raynames.h"

#include "display.h"
#include "lang.h"

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
    Vector3 c2 = (Vector3){cos(get_time())*3, fabs(cos(get_time())*3), sin(get_time())*3};
    if (p.y+10 < Vector3Length(Vector3Subtract(p, c))-1) {
        return (Vector2){p.y+10, 1};
    }
    else {
        float d1 = smoothmin(Vector3Length(Vector3Subtract(p, c2))-1.2, Vector3Length(Vector3Subtract(p, c))-1, 4);
        float d2 = smoothmin(d1, sdOctahedron(p, 3), 2);
        if (d2 < d1) {
            return (Vector2){d2, 2};
        } else {
            return (Vector2){d2, 0};
        }
        
    }
}

float shadow( Vector3 ro, Vector3 rd, float mint, float maxt )
{
    for( float t=mint; t < maxt; )
    {
        // float h = map(ro + rd*t);
        float h = DE(Vector3Add(ro, Vector3Multiply(rd, t))).x;
        if( h<0.001 )
            return 0.0;
        t += h;
    }
    return 1.0;
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
    set_camera_mode(camera, CAMERA_FIRST_PERSON);
    
    while (!window_should_close())
    {
        // display_putc(display, rand()%255, it, it, rand()%32);
        spin+=get_frame_time()*5;
        update_camera(&camera);
        
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
        
        Vector3 cam_pos = camera.position;//(Vector3){sin(get_time())*5, 2, cos(get_time())*5};
        for (int i = 0; i < display->length; i++)
        {
            int _x = i % display->width;
            int _y = i / display->width;
            // display_putc(display, 0x00, _x, _y, 0xaa);
            
            Vector2 q = (Vector2){(float)_x/(float)display->width, (float)_y/(float)display->height};
            Vector2 vpos = (Vector2){-1.0+2.0*q.x, -1.0+2.0*q.y};
            Vector3 vuv = (Vector3){0,1,0};
            Vector3 vrp = camera.target;
            Vector3 prp = cam_pos;
            
            Vector3 vpn = Vector3Normalize(Vector3Subtract(vrp,prp));
            Vector3 u = Vector3Normalize(Vector3CrossProduct(vuv,vpn));
            Vector3 v = Vector3CrossProduct(vpn,u);
            Vector3 vcv = Vector3Add(prp,vpn);
            Vector3 scr_coord = (Vector3){
                vcv.x+vpos.x*u.x*1.0+vpos.y*v.x*1.0,
                vcv.y+vpos.x*u.y*1.0+vpos.y*v.y*1.0,
                vcv.z+vpos.x*u.z*1.0+vpos.y*v.z*1.0
            };
            Vector3 scp = Vector3Normalize(Vector3Subtract(scr_coord,prp));
            
            const Vector3 e = (Vector3){0.02,0,0};
            const float maxd = 50.f;
            Vector2 d = (Vector2){0.02, 0};
            Vector3 c,p,N;
            float f = 1.f;
            Vector3 normal;
            for (int i = 0; i < 256; i++)
            {
                if ((fabs(d.x) < 0.001) || (f > maxd))
                    break;
                
                f += d.x;
                p = Vector3Add(prp, Vector3Multiply(scp, f));
                d = DE(p);
            }
            if (f < maxd) {
                float h = 0.001;
                normal =  Vector3Normalize((Vector3){
                    DE(Vector3Add(p, (Vector3){h, 0, 0})).x - DE(Vector3Subtract(p, (Vector3){h, 0, 0})).x,
                    DE(Vector3Add(p, (Vector3){0, h, 0})).x - DE(Vector3Subtract(p, (Vector3){0, h, 0})).x,
                    DE(Vector3Add(p, (Vector3){0, 0, h})).x - DE(Vector3Subtract(p, (Vector3){0, 0, h})).x,
                });
                
                // phong
                Vector3 light_dir = Vector3Normalize(Vector3Subtract((Vector3){3, 5, 3}, p));
                float intensity = Vector3DotProduct(normal, light_dir);
                intensity = fmax(0, intensity);
                
                if (d.y == 0)
                    display_putc(display, 0xb0+2-(intensity*2), display->width-1-_x, display->height-1-_y, 0x80);
                else if (d.y == 1) {
                    uint8_t c = 0x20;
                    if (fabs(fmodf(floor(p.x)/5, 2)) == 0 || fabs(fmodf(floor(p.z)/5, 2)) == 0) {
                        c = 0x30;
                    }
                    display_putc(display, 0xb0+2-(intensity*2), display->width-1-_x, display->height-1-_y, c);
                }
                else if (d.y == 2)
                    display_putc(display, 0xb0+2-(intensity*2), display->width-1-_x, display->height-1-_y, 0x90);
            } else {
                display_putc(display, 0x00, display->width-1-_x, display->height-1-_y, 0xc0);
            }
        }
        
        display_print(display, format_text("FPS %d", get_fps()), 1, 1, 0x04);
        display_print(display, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 1, 3, 0x0b);
        display_print(display, "1234567890!@#$%^&*()[]\\{}|-=_+,./<>?;:'\"", 1, 4, 0x0b);
        
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
