#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <font.h>
#include <stdint.h>
#include <idt.h>

#if defined(__riscv)
extern void uart_put_char(char c);
extern void uart_printk(char* str);
#endif

int x = 0;
int y = 0;
int ncount = 0;
int current_mode = 0;
int show_crt_window = 0;
int is_window_crt = 0;
int is_button_apps = 0;
int is_button_files = 1;
int is_button_calc = 0;
int draw_0 = 1;
int draw_1 = 1;
int is_scaled = 0;
int is_crushed = 0;
