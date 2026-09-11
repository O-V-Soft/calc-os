#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <stdint.h>

#define KBD_BUFFER_SIZE 32

extern volatile uint8_t last_scancode;

extern uint8_t kbd_buffer[KBD_BUFFER_SIZE];
extern volatile int kbd_head;
extern volatile int kbd_tail;

uint8_t get_scancode();

void input_wait_string(char *buffer);
void input_wait_multiline(char *buffer);
void handle_hotkeys(int code);

void kbd_put_scancode(uint8_t code);

#if defined(__riscv)
void get_string(char *buffer);
#define send_signal(task_id, signal) asm volatile("nop");
#endif

extern int ctrl_pressed;

#endif
