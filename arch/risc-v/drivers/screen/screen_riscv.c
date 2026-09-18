#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <font.h>
#include <stdint.h>
#include <riscv.h>
#include <virtio_gpu.h>

void put_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        VIDEO_MEMORY[y * SCREEN_WIDTH + x] = color;
    }
    flush_gpu();
}

void screen_clear() {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        VIDEO_MEMORY[i] = COLOR_BLACK; 
    }
    flush_gpu();
}

void put_char(char s, uint32_t color) {
    int scale = 1;
    if (is_scaled == 1) scale = 3;
    else if (is_scaled == 2) scale = 2;

    if (s == '\n') {
        x = 0;
        y = y + (8 * scale);
        return;
    }

    if ((uint8_t)s < 32) {
        return; 
    }

    if (x + (8 * scale) > SCREEN_WIDTH) {
        x = 0;
        y = y + (8 * scale);
    }

    if (y + (8 * scale) > SCREEN_HEIGHT) {
        screen_clear();
        x = 0;
        y = 0;
    }

    for (int i = 0; i < 8; i++) {
        for (int v_scale = 0; v_scale < scale; v_scale++) {
            unsigned char bits = font[(uint8_t)s][i];
            
            int current_y = y + (i * scale) + v_scale;
            uint32_t *row = &VIDEO_MEMORY[current_y * SCREEN_WIDTH];

            for (int j = 0; j < 8; j++) {
                if (bits & 0x80) { 
                    for (int h_scale = 0; h_scale < scale; h_scale++) {
                        int current_x = x + (j * scale) + h_scale;
                        if (current_x < SCREEN_WIDTH) {
                            row[current_x] = color; 
                        }
                    }
                }
                bits = bits << 1; 
            }
        }
    }
    
    x = x + (8 * scale);
    flush_gpu();
}

void printk(const char *msg, uint32_t color) {
	for (int i = 0; msg[i] != 0; i++) {
		put_char(msg[i], color);
	}
    flush_gpu();
}

void print(const char *msg, uint32_t color) {
    printk(msg, color);
}

void draw_rect(int x, int y, int width, int height, uint32_t color) {
    for (int y = y; y < y + height; y++) {
        for (int x = x; x < x + width; x++) {
            if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
                VIDEO_MEMORY[y * SCREEN_WIDTH + x] = color;
            }
        }
    }
    flush_gpu();
}

void draw_button(int _x, int _y, int _width, int _height, const char *_msg, uint32_t color, uint32_t text_color) {    
    draw_rect(_x, _y, _width, _height, color);
    
    x = _x + 4;
    y = _y + 4;

    printk(_msg, text_color);
}
