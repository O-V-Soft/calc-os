#if !defined(__riscv)
#include <idt.h>
#include <task.h>
#include <cmos.h>
#include <mouse.h>
#include <utils.h>
#include <idt.h>
#include <task.h>
#endif

#include <keyboard.h>
#include <video.h>
#include <stdint.h>

#if defined(__riscv)
#include <riscv.h>
#endif

int shift_pressed = 0;
int ctrl_pressed = 0;

volatile uint8_t last_scancode = 0;

uint8_t kbd_buffer[KBD_BUFFER_SIZE];
volatile int kbd_head = 0;
volatile int kbd_tail = 0;

void kbd_put_scancode(uint8_t code) {
    int next = (kbd_head + 1) % KBD_BUFFER_SIZE;
    if (next != kbd_tail) { 
        kbd_buffer[kbd_head] = code;
        kbd_head = next;
    }
}

uint8_t get_scancode() {
    cli();
	
    if (kbd_head == kbd_tail) {
        sti();
        return 0; 
    }
    
    uint8_t code = kbd_buffer[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUFFER_SIZE;

    sti();
    return code;
}

void input_wait_string(char *buffer) {
    #if defined(__riscv)
    get_string(buffer);
    #else
    int i = 0;
    int shift_pressed = 0;

    while (1) {
        update_system();

        int code = get_scancode();

        if (ncount == 1) {
            buffer[i] = '\0';
            return;
        }

        if (code == 0) continue;

        if (code == 0x2A || code == 0x36) {
            shift_pressed = 1;
            continue;
        }
        if (code == 0xAA || code == 0xB6) {
            shift_pressed = 0;
            continue;
        }

        if (code == 0x1D) { 
            ctrl_pressed = 1; 
            continue; 
        }

        if (code == 0x9D) { 
            ctrl_pressed = 0; 
            continue; 
        }

        if (code & 0x80) {
            continue;
        }

        handle_hotkeys(code);

        if (code == 0x1C) {
            buffer[i] = '\0';
            return;
        }

        char letter = 0;

        switch (code) {
            case 0x1E: letter = 'a'; break;
            case 0x30: letter = 'b'; break;
            case 0x2E: letter = 'c'; break;
            case 0x20: letter = 'd'; break;
            case 0x12: letter = 'e'; break;
            case 0x21: letter = 'f'; break;
            case 0x22: letter = 'g'; break;
            case 0x23: letter = 'h'; break;
            case 0x17: letter = 'i'; break;
            case 0x24: letter = 'j'; break;
            case 0x25: letter = 'k'; break;
            case 0x26: letter = 'l'; break;
            case 0x32: letter = 'm'; break;
            case 0x31: letter = 'n'; break;
            case 0x18: letter = 'o'; break;
            case 0x19: letter = 'p'; break;
            case 0x10: letter = 'q'; break;
            case 0x13: letter = 'r'; break;
            case 0x1F: letter = 's'; break;
            case 0x14: letter = 't'; break;
            case 0x16: letter = 'u'; break;
            case 0x2F: letter = 'v'; break;
            case 0x11: letter = 'w'; break;
            case 0x2D: letter = 'x'; break;
            case 0x15: letter = 'y'; break;
            case 0x2C: letter = 'z'; break;
            case 0x39: letter = ' '; break;

            case 0x02: letter = shift_pressed ? '!' : '1'; break;
            case 0x03: letter = shift_pressed ? '@' : '2'; break;
            case 0x04: letter = shift_pressed ? '#' : '3'; break;
            case 0x05: letter = shift_pressed ? '$' : '4'; break;
            case 0x06: letter = shift_pressed ? '%' : '5'; break;
            case 0x07: letter = shift_pressed ? '^' : '6'; break;
            case 0x08: letter = shift_pressed ? '&' : '7'; break;
            case 0x09: letter = shift_pressed ? '*' : '8'; break;
            case 0x0A: letter = shift_pressed ? '(' : '9'; break;
            case 0x0B: letter = shift_pressed ? ')' : '0'; break;
            case 0x0C: letter = shift_pressed ? '_' : '-'; break;
            case 0x0D: letter = shift_pressed ? '+' : '='; break;
            case 0x34: letter = shift_pressed ? '>' : '.'; break;
            case 0x35: letter = shift_pressed ? '?' : '/'; break;
            case 0x1A: letter = shift_pressed ? '{' : '['; break;
            case 0x1B: letter = shift_pressed ? '}' : ']'; break;
            case 0x33: letter = shift_pressed ? '<' : ','; break;
            case 0x28: letter = shift_pressed ? '"' : '\''; break;
            case 0x27: letter = shift_pressed ? ':' : ';'; break;

            case 0x0E: 
                if (i > 0) {
                    i--;
                    if (is_window_crt == 0 && current_mode == 0) {
                        if (x > 32) { 
                            x -= 16; 
                            draw_rect(x, y, 16, 16, COLOR_BLACK); 
                        }
                    } else {
                        if (x > 48) {
                            x -= 8; 
                            draw_rect(x, y, 8, 8, COLOR_BLACK); 
                        }
                    }
                }
                break;

            default:
                letter = 0;
                break;
        }

        if (shift_pressed && letter >= 'a' && letter <= 'z') {
            letter = letter - 'a' + 'A';
        }

        if (letter != 0 && i < 255) {
            put_char(letter, COLOR_WHITE);
            buffer[i] = letter;
            i++;
        }
    }
    #endif
}

void input_wait_multiline(char *buffer) {
#if defined(__riscv)
    get_string(buffer);
#else
    int i = 0;
    int shift_pressed = 0;
    int ctrl_pressed = 0;

    while (1) {
        update_system();

        int code = get_scancode();

        if (code == 0) continue;

        if (code == 0x2A || code == 0x36) { 
			shift_pressed = 1; 
			continue; 
		}
		
        if (code == 0xAA || code == 0xB6) { 
			shift_pressed = 0; 
			continue; 
		}
			
        if (code == 0x1D) { 
			ctrl_pressed = 1; 
			continue; 
		}
			
        if (code == 0x9D) { 
			ctrl_pressed = 0; 
			continue; 
		}

        if (code & 0x80) continue;

        if (code == 0x01) {
            buffer[i] = '\0';
            return;
        }

        if (code == 0x1C) {
            if (i < 255) {
                buffer[i++] = '\n';
                put_char('\n', COLOR_WHITE);
            }
            continue;
        }

        char letter = 0;

        switch (code) {
            case 0x1E: letter = 'a'; break;
            case 0x30: letter = 'b'; break;
            case 0x2E: letter = 'c'; break;
            case 0x20: letter = 'd'; break;
            case 0x12: letter = 'e'; break;
            case 0x21: letter = 'f'; break;
            case 0x22: letter = 'g'; break;
            case 0x23: letter = 'h'; break;
            case 0x17: letter = 'i'; break;
            case 0x24: letter = 'j'; break;
            case 0x25: letter = 'k'; break;
            case 0x26: letter = 'l'; break;
            case 0x32: letter = 'm'; break;
            case 0x31: letter = 'n'; break;
            case 0x18: letter = 'o'; break;
            case 0x19: letter = 'p'; break;
            case 0x10: letter = 'q'; break;
            case 0x13: letter = 'r'; break;
            case 0x1F: letter = 's'; break;
            case 0x14: letter = 't'; break;
            case 0x16: letter = 'u'; break;
            case 0x2F: letter = 'v'; break;
            case 0x11: letter = 'w'; break;
            case 0x2D: letter = 'x'; break;
            case 0x15: letter = 'y'; break;
            case 0x2C: letter = 'z'; break;
            case 0x39: letter = ' '; break;

            case 0x02: letter = shift_pressed ? '!' : '1'; break;
            case 0x03: letter = shift_pressed ? '@' : '2'; break;
            case 0x04: letter = shift_pressed ? '#' : '3'; break;
            case 0x05: letter = shift_pressed ? '$' : '4'; break;
            case 0x06: letter = shift_pressed ? '%' : '5'; break;
            case 0x07: letter = shift_pressed ? '^' : '6'; break;
            case 0x08: letter = shift_pressed ? '&' : '7'; break;
            case 0x09: letter = shift_pressed ? '*' : '8'; break;
            case 0x0A: letter = shift_pressed ? '(' : '9'; break;
            case 0x0B: letter = shift_pressed ? ')' : '0'; break;
            case 0x0C: letter = shift_pressed ? '_' : '-'; break;
            case 0x0D: letter = shift_pressed ? '+' : '='; break;
            case 0x34: letter = shift_pressed ? '>' : '.'; break;
            case 0x35: letter = shift_pressed ? '?' : '/'; break;
            case 0x1A: letter = shift_pressed ? '{' : '['; break;
            case 0x1B: letter = shift_pressed ? '}' : ']'; break;
            case 0x33: letter = shift_pressed ? '<' : ','; break;
            case 0x28: letter = shift_pressed ? '"' : '\''; break;
            case 0x27: letter = shift_pressed ? ':' : ';'; break;

            case 0x0E: 
                if (i > 0) {
                    i--;
                    if (is_window_crt == 0 && current_mode == 0) {
                        if (x > 32) { 
							x -= 16; 
							draw_rect(x, y, 16, 16, COLOR_BLACK); 
						}
                    } else {
                        if (x > 48) { 
							x -= 8; 
							draw_rect(x, y, 8, 8, COLOR_BLACK); 
						}
                    }
                }
                break;

            default:
                letter = 0;
                break;
        }

        if (shift_pressed && letter >= 'a' && letter <= 'z') {
            letter = letter - 'a' + 'A';
        }

        if (letter != 0 && i < 255) {
            put_char(letter, COLOR_WHITE);
            buffer[i] = letter;
            i++;
        }
    }
#endif
}
