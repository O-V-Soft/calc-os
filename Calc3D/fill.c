#include <video.h>
#include <mouse.h>
#include <stdint.h>

#if defined(__riscv)
#include <riscv.h>
#else
#include <idt.h>
#include <utils.h>
#endif

int interpolate_x(int y, int x1, int y1, int x2, int y2) {
    if (y1 == y2) return x1;
    return x1 + ((y - y1) * (x2 - x1)) / (y2 - y1);
}

void draw_h_line(int x1, int x2, int y, uint8_t color) {
    if (x1 > x2) { 
        int t = x1; 
        x1 = x2; 
        x2 = t; 
    }

    for (int x = x1; x <= x2; x++) {
        put_pixel(x, y, color);
    }
}

void draw_filled_triangle(Point2D p1, Point2D p2, Point2D p3, uint8_t color) {
    if (p1.y > p2.y) {
        Point2D t = p1; 
        p1 = p2; 
        p2 = t; 
    }

    if (p1.y > p3.y) {
        Point2D t = p1; 
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

        draw_h_line(x_long, x_short, y, color);
    }

    for (int y = p2.y; y <= p3.y; y++) {
        int x_long = interpolate_x(y, p1.x, p1.y, p3.x, p3.y);

        int x_short = interpolate_x(y, p2.x, p2.y, p3.x, p3.y);

        draw_h_line(x_long, x_short, y, color);
    }
}
