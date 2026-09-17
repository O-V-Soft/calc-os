#include <video.h>
#include <mouse.h>
#include <stdint.h>

#if defined(__riscv)
#include <riscv.h>
#else
#include <idt.h>
#include <utils.h>
#endif

void init_palette() {
#if defined(__riscv)
    return; 
#else
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
#endif
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
