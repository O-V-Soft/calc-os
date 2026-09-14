#include <video.h>
#include <mouse.h>
#include <stdint.h>

#if defined(__riscv)
#include <riscv.h>
#else
#include <idt.h>
#include <utils.h>
#endif

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
