#ifndef VIDEO_H
#define VIDEO_H
#include <stdint.h>

#define fb_address (*(volatile uint32_t*)0x0500)
#define VIDEO_MEMORY ((uint8_t*)(uint32_t)fb_address)

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768

#define COLOR_BLACK          0
#define COLOR_BLUE           1
#define COLOR_GREEN          2
#define COLOR_CYAN           3
#define COLOR_RED            4
#define COLOR_MAGENTA        5
#define COLOR_BROWN          6
#define COLOR_LIGHT_GRAY     7
#define COLOR_DARK_GRAY      8
#define COLOR_LIGHT_BLUE     9
#define COLOR_LIGHT_GREEN    10
#define COLOR_LIGHT_CYAN     11
#define COLOR_LIGHT_RED      12
#define COLOR_LIGHT_MAGENTA  13
#define COLOR_YELLOW         14
#define COLOR_WHITE          15

#define COLOR_SYS_BG         16  
#define COLOR_SYS_TITLE      17  
#define COLOR_SYS_SHADOW     18  
#define COLOR_SYS_LIGHT      19  
#define COLOR_ACCENT_BLUE    20  
#define COLOR_TEXT_DARK      21 
#define COLOR_ORANGE         22  

extern int x;
extern int y;
extern int ncount;
extern int current_mode;
extern int show_crt_window;
extern int is_window_crt;
extern int mouse_x;
extern int mouse_y;
extern int mouse_left_button;
extern int is_button_apps;
extern int is_button_files;
extern int is_button_calc;
extern int draw_0;
extern int draw_1;
extern int is_scaled;
extern char content[512];
extern int current_uid;
extern int win_file_x;
extern int win_file_y;
extern int is_crushed;

extern char name[128];
extern char content[512];
extern const char* root_password;

void draw_desktop();

void screen_clear();
void put_char(char s, uint8_t color);
void printk(const char *msg, uint8_t color);
void print(const char *msg, uint8_t color);
void draw_rect(int x, int y, int width, int height, uint8_t color);
void draw_rounded_rect(int x, int y, int width, int height, int r, uint8_t color);

void update_system();
void handle_hotkeys(int code);
void init_palette();

extern void outb(uint16_t port, uint8_t val);
extern uint8_t inb(uint16_t port);
extern void outw(uint16_t port, uint16_t val);
extern uint16_t inw(uint16_t port);
extern void outl(uint16_t port, uint32_t val);
extern uint32_t inl(uint16_t port);
void graphics();

#endif
