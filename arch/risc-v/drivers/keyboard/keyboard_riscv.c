#include <keyboard.h>
#include <video.h>
#include <stdint.h>
#include <riscv.h>

void handle_hotkeys(int code) {
	if (code == 0x4B) {
		if (current_mode != 0) {
			is_scaled = 0;
			current_mode = 0;
			is_window_crt = 0;
			show_crt_window = 0;
			ncount = 1;
		}
	}
	
	if (code == 0x4D) {
		if (current_mode == 0) {
		    is_scaled = 0;
		    current_mode = 1;
		    ncount = 1;
		}
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
}

void get_string(char *buffer) {
    int i = 0;

    while (1) {
        char c = uart_get_char(); 
        if (c == 0) continue;

        if (c == '\r' || c == '\n') {
            buffer[i] = '\0';
            put_char('\n', COLOR_WHITE); 
            return;
        }

    	if (handle_ansi(c)) {
            continue; 
        }

        if (c == 0x08 || c == 0x7F) {
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
            continue;
        }

        if ((uint8_t)c < 32) {
            continue;
        }

        if (i < 255) {
            put_char(c, COLOR_WHITE); 
            buffer[i] = c;
            i++;
        }
    }
}
