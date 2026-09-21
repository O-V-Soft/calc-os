#include <video.h>
#include <mouse.h>
#include <stdint.h>

#if defined(__riscv)
#include <riscv.h>
#else
#include <idt.h>
#include <utils.h>
#endif

#define MAX_PIXELS 4

const uint8_t texture[MAX_PIXELS * MAX_PIXELS] = {
    COLOR_DARK_GRAY, COLOR_DARK_GRAY, COLOR_DARK_GRAY, COLOR_DARK_GRAY,
    COLOR_DARK, COLOR_DARK, COLOR_DARK, COLOR_DARK, 
    COLOR_DARK, COLOR_DARK, COLOR_DARK, COLOR_DARK, 
    COLOR_DARK_GRAY, COLOR_DARK_GRAY, COLOR_DARK_GRAY, COLOR_DARK_GRAY
};

void draw_textured_box(int x_min, int y_min, int x_max, int y_max) {
    int width = x_max - x_min;
    int height = y_max - y_min;

    if (width <= 0 || height <= 0) return;

    for (int y = y_min; y <= y_max; y++) {
        int tex_y = ((y - y_min) * MAX_PIXELS) / height;

        for (int x = x_min; x <= x_max; x++) {
            int tex_x = ((x - x_min) * MAX_PIXELS) / width;

            uint8_t color = texture[tex_y * MAX_PIXELS + tex_x];

            put_pixel(x, y, color);
        }
    }
}
