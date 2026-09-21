#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <font.h>
#include <stdint.h>
#include <idt.h>

void screen_clear() {
    memset(VIDEO_MEMORY, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
    x = 0;
    y = 0;
    ncount = 1;
    is_scaled = 0;
}

void set_palette_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    outb(0x03C8, index);    
    outb(0x03C9, r >> 2);   
    outb(0x03C9, g >> 2);    
    outb(0x03C9, b >> 2);   
}

void init_palette() {
    set_palette_color(0, 0, 0, 0);         
    set_palette_color(1, 0, 0, 170);      
    set_palette_color(2, 0, 170, 0);      
    set_palette_color(4, 170, 0, 0);       
    set_palette_color(7, 170, 170, 170);   
    set_palette_color(8, 85, 85, 85);       
    set_palette_color(14, 255, 255, 85);  
    set_palette_color(15, 255, 255, 255);   

    set_palette_color(3, 0, 170, 170);     
    set_palette_color(5, 170, 0, 170);     
    set_palette_color(6, 100, 50, 0);       
    set_palette_color(9, 85, 85, 255);     
    set_palette_color(10, 85, 255, 85);    
    set_palette_color(11, 85, 255, 255);   
    set_palette_color(12, 255, 85, 85);     
    set_palette_color(13, 255, 85, 255);    

    set_palette_color(16, 212, 208, 200); 
    set_palette_color(17, 10, 24, 80);      
    set_palette_color(18, 128, 128, 128); 
    set_palette_color(19, 230, 230, 230);
    
    set_palette_color(20, 0, 120, 215);    
    set_palette_color(21, 26, 26, 26);     
    set_palette_color(22, 255, 165, 0); 
}

void put_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        VIDEO_MEMORY[y * SCREEN_WIDTH + x] = color;
    }
}

void put_char(char s, uint8_t color) {
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
        y = 0;
    }

    for (int i = 0; i < 8; i++) {
        for (int v_scale = 0; v_scale < scale; v_scale++) {
            unsigned char bits = font[(uint8_t)s][i];
            
            int current_y = y + (i * scale) + v_scale;
            uint8_t *row = &VIDEO_MEMORY[current_y * SCREEN_WIDTH];

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
}

void printk(const char *msg, uint8_t color) {
	for (int i = 0; msg[i] != 0; i++) {
		put_char(msg[i], color);
	}
}

void print(const char *msg, uint8_t color) {
#if defined(__riscv)
    printk(msg, color);
#else
    sys_write(1, msg, color);
#endif
}

void draw_button(int _x, int _y, int _width, int _height, const char *_msg, uint8_t color, uint8_t text_color) {
    int radius = 4;
    
    draw_rounded_rect(_x, _y, _width, _height, radius, color);
    
    x = _x + 4;
    y = _y + 4;

    printk(_msg, text_color);
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

void draw_rect(int x, int y, int width, int height, uint8_t color) {
    for (int i = 0; i < height; i++) {
        int curr_y = y + i;
        if (curr_y >= 0 && curr_y < SCREEN_HEIGHT) {
            int start_x = (x < 0) ? 0 : x;
            int end_x = (x + width > SCREEN_WIDTH) ? SCREEN_WIDTH : (x + width);
            if (end_x > start_x) {
                memset(&VIDEO_MEMORY[curr_y * SCREEN_WIDTH + start_x], color, end_x - start_x);
            }
        }
    }
}

void draw_rounded_rect(int x, int y, int width, int height, int r, uint8_t color) {
    draw_rect(x + r, y, width - 2 * r, height, color);
    draw_rect(x, y + r, width, height - 2 * r, color);

    for (int dy = 0; dy <= r; dy++) {
        for (int dx = 0; dx <= r; dx++) {
            if (dx * dx + dy * dy <= r * r) {
                int px1 = x + r - dx;
                int py1 = y + r - dy;
                if (px1 >= 0 && px1 < SCREEN_WIDTH && py1 >= 0 && py1 < SCREEN_HEIGHT) {
                    VIDEO_MEMORY[py1 * SCREEN_WIDTH + px1] = color;
                }

                int px2 = x + width - 1 - r + dx;
                int py2 = y + r - dy;
                if (px2 >= 0 && px2 < SCREEN_WIDTH && py2 >= 0 && py2 < SCREEN_HEIGHT) {
                    VIDEO_MEMORY[py2 * SCREEN_WIDTH + px2] = color;
                }

                int px3 = x + r - dx;
                int py3 = y + height - 1 - r + dy;
                if (px3 >= 0 && px3 < SCREEN_WIDTH && py3 >= 0 && py3 < SCREEN_HEIGHT) {
                    VIDEO_MEMORY[py3 * SCREEN_WIDTH + px3] = color;
                }   

                int px4 = x + width - 1 - r + dx;
                int py4 = y + height - 1 - r + dy;
                if (px4 >= 0 && px4 < SCREEN_WIDTH && py4 >= 0 && py4 < SCREEN_HEIGHT) {
                    VIDEO_MEMORY[py4 * SCREEN_WIDTH + px4] = color;
                }
            }
        }
    }
}

void draw_line(int x0, int y0, int x1, int y1, uint8_t color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx - dy;

    while (1) {
        if (x0 >= 0 && x0 < SCREEN_WIDTH && y0 >= 0 && y0 < SCREEN_HEIGHT) {
            VIDEO_MEMORY[y0 * SCREEN_WIDTH + x0] = color;
        }

        if (x0 == x1 && y0 == y1) {
            break;
        }

        int err2 = 2 * err;
        if (err2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (err2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}
