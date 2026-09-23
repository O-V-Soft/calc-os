#include <video.h>
#include <mouse.h>
#include <stdint.h>

#if defined(__riscv)
#include <riscv.h>
#else
#include <idt.h>
#include <utils.h>
#endif

#define TEX_SIZE 4

#define B COLOR_WHITE      
#define G COLOR_LIGHT_GRAY  

const uint8_t texture[TEX_SIZE * TEX_SIZE] = {
    B, B, G, G,
    B, B, B, G,
    B, B, B, G,
    B, B, B, G
};

void draw_textured_line(int x1, int x2, int y, int x_min, int x_max, int y_min, int y_max) {
    if (x1 > x2) { 
        int t = x1; x1 = x2; x2 = t; 
    }

    int total_w = x_max - x_min;
    int total_h = y_max - y_min;
    if (total_w <= 0) total_w = 1;
    if (total_h <= 0) total_h = 1;

    int tex_y = ((y - y_min) * (TEX_SIZE - 1)) / total_h;
    if (tex_y >= TEX_SIZE) tex_y = TEX_SIZE - 1;

    for (int x = x1; x <= x2; x++) {
        int tex_x = ((x - x_min) * (TEX_SIZE - 1)) / total_w;
        if (tex_x < 0) tex_x = 0;
        if (tex_x >= TEX_SIZE) tex_x = TEX_SIZE - 1;

        uint8_t color = texture[tex_y * TEX_SIZE + tex_x];
        put_pixel(x, y, color);
    }
}

void draw_textured_triangle(Point2D p1, Point2D p2, Point2D p3, int x_min, int x_max, int y_min, int y_max) {
    if (p1.y > p2.y) { 
        Point2D t = p1;
        p1 = p2; 
        p2 = t; 
    }
    
    if (p1.y > p3.y) { 
        Point2D 
        t = p1; 
        p1 = p3; 
        p3 = t; 
    }
    
    if (p2.y > p3.y) { 
        Point2D t = p2; 
        p2 = p3; 
        p3 = t; 
    }

    if (p1.y == p3.y) return;

    for (int y = p1.y; y <= p2.y; y++) {
        int x_long = interpolate_x(y, p1.x, p1.y, p3.x, p3.y);
        int x_short = interpolate_x(y, p1.x, p1.y, p2.x, p2.y);

        draw_textured_line(x_long, x_short, y, x_min, x_max, y_min, y_max);
    }

    for (int y = p2.y; y <= p3.y; y++) {
        int x_long = interpolate_x(y, p1.x, p1.y, p3.x, p3.y);
        int x_short = interpolate_x(y, p2.x, p2.y, p3.x, p3.y);

        draw_textured_line(x_long, x_short, y, x_min, x_max, y_min, y_max);
    }
}
