#ifndef MOUSE_H
#define MOUSE_H
#include <stdint.h>

extern void mouse_wait();
extern void mouse_wait_input();
extern void init_mouse();
uint8_t get_scancode();

extern int mouse_x;
extern int mouse_y;
extern int old_mouse_x;
extern int old_mouse_y;
extern int mouse_left_button;
extern uint8_t mouse_cursor[16][16];
extern uint8_t cursor_back_buffer[256];

extern volatile uint8_t mouse_packet[3];
extern volatile int mouse_ready;
extern volatile int mouse_cycle;

void mouse_handler();
void draw_cursor(int mx, int my);
void save_background();
void restore_background();
void check_ui_clicks();

void update_system();

#endif