#include <video.h>
#include <cmos.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <stdint.h>
#include <idt.h>

volatile int mouse_cycle = 0;
volatile uint8_t mouse_packet[3];
volatile int mouse_ready = 0;

unsigned char mouse_cursor[16][16] = {
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0},
        {1,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0},
        {1,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0},
        {1,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0},
        {1,2,2,2,2,2,2,2,1,0,0,0,0,0,0,0},
        {1,2,2,2,2,2,2,2,2,1,0,0,0,0,0,0},
        {1,2,2,2,2,2,2,2,2,2,1,0,0,0,0,0},
        {1,2,2,2,2,2,2,1,1,1,1,0,0,0,0,0},
        {1,2,2,2,1,2,2,1,0,0,0,0,0,0,0,0},
        {1,2,2,1,0,1,2,2,1,0,0,0,0,0,0,0},
        {1,2,1,0,0,1,2,2,1,0,0,0,0,0,0,0},
        {1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0},
};

void stub_mouse_handler() {
    uint8_t status = inb(0x64);
    
    if ((status & 0x01) && (status & 0x20)) {
        uint8_t data = inb(0x60);
        
        if (mouse_cycle == 0 && !(data & 0x08)) {
            mouse_cycle = 0;
        } else {
            mouse_packet[mouse_cycle++] = data;
            
            if (mouse_cycle == 3) {
                mouse_cycle = 0;
                mouse_ready = 1;
            }
        }
    }

    outb(0xA0, 0x20); 
    outb(0x20, 0x20); 
}

void draw_cursor(int mx, int my) {
    for (int row = 0; row < 16; row = row + 1) {
        for (int col = 0; col < 16; col = col + 1) {
            uint8_t c = mouse_cursor[row][col];
            
            if (c == 1 || c == 2) {
                int curr_x = mx + col;
                int curr_y = my + row;

                if (curr_x >= 0 && curr_x < SCREEN_WIDTH && curr_y >= 0 && curr_y < SCREEN_HEIGHT) {
                    uint8_t *ptr = &VIDEO_MEMORY[curr_y * SCREEN_WIDTH + curr_x];

                    if (c == 1) {
                        *ptr = 15; 
                    } else {
                        *ptr = 0; 
                    }
                }
            }
        }
    }
}

int mouse_x = 512;
int mouse_y = 384;

int old_mouse_x = 512;
int old_mouse_y = 384;
int mouse_left_button = 0;

uint8_t cursor_back_buffer[256];

void mouse_handler() {
    if (!mouse_ready) return; 
    mouse_ready = 0;

    uint8_t first_byte = mouse_packet[0];
    if (!(first_byte & 0x08)) return; 

    int x_offset = (int)((signed char)mouse_packet[1]);
    int y_offset = (int)((signed char)mouse_packet[2]);

    mouse_x += x_offset;
    mouse_y -= y_offset;

    if (mouse_x < 0) mouse_x = 0;
    if (mouse_y < 0) mouse_y = 0;
    if (mouse_x > (SCREEN_WIDTH - 16))  mouse_x = SCREEN_WIDTH - 16; 
    if (mouse_y > (SCREEN_HEIGHT - 16)) mouse_y = SCREEN_HEIGHT - 16; 

    mouse_left_button = first_byte & 0x01;
}

void save_background() {
    for (int i = 0; i < 16; i = i + 1) {
        for (int j = 0; j < 16; j = j + 1) {
            int cur_x = mouse_x + j;
            int cur_y = mouse_y + i;

            if (cur_x >= 0 && cur_x < SCREEN_WIDTH && cur_y >= 0 && cur_y < SCREEN_HEIGHT) {
                cursor_back_buffer[i * 16 + j] = VIDEO_MEMORY[cur_y * SCREEN_WIDTH + cur_x];
            }
        }
    }
}

void restore_background() {
    for (int i = 0; i < 16; i = i + 1) {
        for (int j = 0; j < 16; j = j + 1) {
            int old_x = old_mouse_x + j;
            int old_y = old_mouse_y + i;

            if (old_x >= 0 && old_x < SCREEN_WIDTH && old_y >= 0 && old_y < SCREEN_HEIGHT) {
                VIDEO_MEMORY[old_y * SCREEN_WIDTH + old_x] = cursor_back_buffer[i * 16 + j];
            }
        }
    }
}

void check_ui_clicks() {
    if (current_mode == 2 && mouse_left_button == 1) {
        if (mouse_x >= win_file_x && mouse_x <= (win_file_x + 432) && mouse_y >= win_file_y && mouse_y <= (win_file_y + 32)) {
            
            win_file_x = mouse_x - 200;
            win_file_y = mouse_y - 16;
            
            if (win_file_x < 0) win_file_x = 0;
            if (win_file_y < 40) win_file_y = 40;
            if (win_file_x > (1024 - 432)) win_file_x = 1024 - 432;
            if (win_file_y > (768 - 260)) win_file_y = 768 - 260;

            ncount = 1; 
        }
    }
    if (mouse_left_button == 1) {
        if (mouse_x >= 238 && mouse_x <= 374 && mouse_y >= 5 && mouse_y <= 31) {
            is_scaled = 0;

            if (current_mode != 1) {
                current_mode = 1;
                ncount = 1;
            }
        }
        
        if (mouse_x >= 78 && mouse_x <= 214 && mouse_y >= 5 && mouse_y <= 31) {
            is_scaled = 0;

            if (current_mode != 0) {
                current_mode = 0;
                ncount = 1;
            }
        }

        if (current_mode == 1 && mouse_x >= 352 && mouse_x <= 672 && mouse_y >= 250 && mouse_y <= 286) {
            current_mode = 2;
            ncount = 1;
        }

        if (current_mode == 1 && mouse_x >= 352 && mouse_x <= 672 && mouse_y >= 350 && mouse_y <= 386) {
            current_mode = 3;
            ncount = 1;
        }

        if (current_mode == 2 && mouse_y >= 728 && mouse_y <= SCREEN_HEIGHT) {
            show_crt_window = 1;
            ncount = 1;
        }

        if (mouse_x >= 10 && mouse_x <= 66 && mouse_y >= 5 && mouse_y <= 31) {
            is_scaled = 0;
            is_button_calc = 1;
            ncount = 1;
        }

        if (mouse_x >= 10 && mouse_x <= 80 && mouse_y >= 31 && mouse_y <= 57) {
            current_mode = 0;
            draw_0 = 1;
            ncount = 1;
        }

        if (mouse_x >= 10 && mouse_x <= 80 && mouse_y >= 51 && mouse_y <= 77) {
            is_scaled = 0;
            current_mode = 1;
            draw_1 = 1;
            ncount = 1;
        }

        if (mouse_x >= 10 && mouse_x <= 25 && mouse_y >= 85 && mouse_y <= 100) {
            is_scaled = 0;
            is_button_calc = 0;
            ncount = 1;
        }

        if (mouse_x >= 191 && mouse_x <= 206 && mouse_y >= 11 && mouse_y <= 26) { 
            draw_0 = 0;
            ncount = 1;
        }

        if (mouse_x >= 351 && mouse_x <= 366 && mouse_y >= 5 && mouse_y <= 20) {
            is_scaled = 0;
            draw_1 = 4; 
            ncount = 1;
        }

        if (mouse_x >= 65 && mouse_x <= 80 && mouse_y >= 85 && mouse_y <= 100) {
            reboot();
        }
    }
}

void update_system() {
    mouse_handler();
    if (mouse_x != old_mouse_x || mouse_y != old_mouse_y) {
        restore_background();
        save_background();
        draw_cursor(mouse_x, mouse_y);
        old_mouse_x = mouse_x;
        old_mouse_y = mouse_y;
    }
    check_ui_clicks();
}