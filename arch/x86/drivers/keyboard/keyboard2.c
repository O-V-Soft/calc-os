#include <idt.h>
#include <task.h>
#include <cmos.h>
#include <mouse.h>
#include <utils.h>
#include <idt.h>
#include <task.h>
#include <keyboard.h>
#include <video.h>
#include <stdint.h>

void keyboard_handler() {
    uint8_t status = inb(0x64);

    if (status & 0x01) {
        uint8_t scancode = inb(0x60); 
        
        if (!(status & 0x20)) {
            kbd_put_scancode(scancode);
        }
    }

    outb(0x20, 0x20);
}

void handle_hotkeys(int code) {
	if (code == 0x4B) {
		is_scaled = 0;
		current_mode = 0;
		is_window_crt = 0;
		show_crt_window = 0;
		ncount = 1;
	}
	if (code == 0x4D) {
		is_scaled = 0;
		current_mode = 1;
		ncount = 1;
	}

	if (code == 0x3C && current_mode == 1) {
		show_crt_window = 1;
		is_window_crt = 0;
		ncount = 1;
	}

	if (code == 0x1C && current_mode == 1 && is_button_files == 1) {
		current_mode = 2;
		ncount = 1;
	}
	if (code == 0x50 && current_mode == 1 && is_button_files == 1) {
		is_button_apps = 1;
		is_button_files = 0;
		ncount = 1;
	}
	if (code == 0x1C && current_mode == 1 && is_button_apps == 1) {
		current_mode = 3;
		ncount = 1;
	}
	if (code == 0x48 && current_mode == 1 && is_button_apps == 1) {
		is_button_apps = 0;
		is_button_files = 1;
		current_mode = 1;
		ncount = 1;
	}
	if (code == 0x5B) {
		is_scaled = 0;
		is_button_calc = 1;
		ncount = 1;
	}
    if (code == 0x2E && ctrl_pressed == 1) { 
        if (current_uid != 0) {
            if (task_list[2].is_active) {
                send_signal(2, SIGINT); 
                return;
            }
        } else {
            if (task_list[2].is_active) {
                send_signal(2, SIGINT); 
            } else {
                send_signal(0, SIGINT); 
			}
			
            return;
        }
	}
}
