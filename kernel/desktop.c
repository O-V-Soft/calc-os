#include <fat.h>
#include <cmos.h>
#include <video.h>
#include <utils.h>
#include <mouse.h>
#include <keyboard.h>
#include <idt.h>
#include <stdint.h>
#include <ata.h>
#include <sound.h>
#include <pci.h>
#include <mm.h>
#include <forth.h>
#include <task.h>
#include <vfs.h>
#include <casm.h>
#include <coms.h>
#include <font.h>

#if defined(__riscv)
#include <riscv.h>
#endif

static const uint8_t fade_palette[24] = {
    COLOR_WHITE, COLOR_WHITE, COLOR_SYS_LIGHT, 
    COLOR_SYS_LIGHT, COLOR_SYS_LIGHT, COLOR_SYS_LIGHT, 
    COLOR_LIGHT_GRAY,  COLOR_LIGHT_GRAY, 
    COLOR_LIGHT_GRAY,  COLOR_LIGHT_GRAY, 
    COLOR_SYS_SHADOW,  COLOR_SYS_SHADOW, 
    COLOR_SYS_SHADOW,  COLOR_DARK_GRAY, 
    COLOR_DARK_GRAY,  COLOR_DARK_GRAY, 
    COLOR_ACCENT_BLUE,  COLOR_ACCENT_BLUE, 
    COLOR_ACCENT_BLUE,  COLOR_ACCENT_BLUE,
    COLOR_ACCENT_BLUE,  COLOR_ACCENT_BLUE,  
    COLOR_ACCENT_BLUE,  COLOR_ACCENT_BLUE
};

void draw_desktop() {
#if defined(__riscv)
    return; 
#else
    draw_rect(0, 40, 1024, 728, COLOR_ACCENT_BLUE);

    int glass_x = 60;
    int glass_y = 180;
    int glass_w = 500;
    int glass_h = 320;

    draw_rect(glass_x, glass_y, glass_w, 2, COLOR_WHITE);
    draw_rect(glass_x, glass_y, 2, glass_h, COLOR_WHITE);
    draw_rect(glass_x, glass_y + glass_h, glass_w, 2, COLOR_SYS_SHADOW);
    draw_rect(glass_x + glass_w, glass_y, 2, glass_h, COLOR_SYS_SHADOW);

    draw_rect(glass_x + 6, glass_y + glass_h + 6, glass_w, 4, COLOR_BLACK); 
    draw_rect(glass_x + glass_w + 6, glass_y + 6, 4, glass_h, COLOR_BLACK);

    char *text = "CalcOS";
    int text_x = glass_x + 40;
    int text_y = glass_y + 80;
    int scale = 3;

    is_scaled = 1;
    x = text_x + 3; y = text_y + 3;
    print(text, COLOR_BLACK);

    x = text_x; y = text_y;
    print(text, COLOR_WHITE);
    is_scaled = 0;

    int reflect_y = text_y + 24 + 2; 

    cli();
    for (int char_idx = 0; text[char_idx] != 0; char_idx++) {
        unsigned char s = (unsigned char)text[char_idx];
        int start_x = text_x + (char_idx * 8 * scale);

        for (int i = 0; i < 8; i++) {
            unsigned char bits = font[s][7 - i];

            for (int v_scale = 0; v_scale < scale; v_scale++) {
                int current_pixel_y = reflect_y + (i * scale) + v_scale;
                
                if (current_pixel_y >= SCREEN_HEIGHT || current_pixel_y >= (glass_y + glass_h)) {
                    continue;
                }

                int fade_index = (i * scale) + v_scale;
                uint8_t reflect_color = (fade_index < 24) ? fade_palette[fade_index] : 20;

                uint8_t *row = &VIDEO_MEMORY[current_pixel_y * SCREEN_WIDTH];
                unsigned char bits_copy = bits;

                for (int j = 0; j < 8; j++) {
                    if (bits_copy & 0x80) {
                        for (int h_scale = 0; h_scale < scale; h_scale++) {
                            int current_pixel_x = start_x + (j * scale) + h_scale;
                            
                            if (current_pixel_x >= 0 && current_pixel_x < SCREEN_WIDTH) {
                                row[current_pixel_x] = reflect_color;
                            }
                        }
                    }
                    bits_copy = bits_copy << 1;
                }
            }
        }
    }
    sti();
#endif
}

void graphics() {
    #if defined(__riscv)
    return;
    #else
    cli();
    if (current_mode == 0) {
        if (draw_0 == 1) {
            draw_rect(0, 40, 1024, 728, COLOR_BLACK);

            if (draw_0 == 1) {
                draw_button(78, 5, 136, 26, "Terminal", COLOR_RED, COLOR_WHITE);
                draw_button(191, 11, 15, 15, "x", COLOR_WHITE, COLOR_BLACK);
            }

            if (draw_1 == 1) {
                draw_button(238, 5, 136, 26, "Explorer", COLOR_GREEN, COLOR_WHITE);
                draw_button(351, 11, 15, 15, "x", COLOR_WHITE, COLOR_BLACK);
            }

            if (is_button_calc == 1) {
                draw_rect(10, 31, 72, 70, COLOR_LIGHT_GRAY);

                draw_button(10, 31, 70, 26, "Terminal", COLOR_RED, COLOR_WHITE);
                draw_button(10, 51, 70, 26, "Explorer", COLOR_GREEN, COLOR_WHITE);

                draw_button(10, 85, 15, 15, "x", COLOR_BLACK, COLOR_WHITE);
                draw_button(65, 85, 15, 15, "r", COLOR_BLACK, COLOR_WHITE);
            }

            x = 0;
            y = 50;

        } else {
            current_mode = 5;
            ncount = 1;
        }
    }
    else if (current_mode == 2) {
        draw_desktop();

        if (draw_0 == 1) {
            draw_button(78, 5, 136, 26, "Terminal", COLOR_GREEN, COLOR_WHITE);
            draw_button(191, 11, 15, 15, "x", COLOR_WHITE, COLOR_BLACK);
        }

        if (draw_1 == 1) {
            draw_button(238, 5, 136, 26, "Explorer", COLOR_RED, COLOR_WHITE);
            draw_button(351, 11, 15, 15, "x", COLOR_WHITE, COLOR_BLACK);
        }

        if (is_button_calc == 1) {
            draw_rect(10, 31, 72, 70, COLOR_LIGHT_GRAY);

            draw_button(10, 31, 70, 26, "Terminal", COLOR_GREEN, COLOR_WHITE);
            draw_button(10, 51, 70, 26, "Explorer", COLOR_RED, COLOR_WHITE);

            draw_button(10, 85, 15, 15, "x", COLOR_BLACK, COLOR_WHITE);
            draw_button(65, 85, 15, 15, "r", COLOR_BLACK, COLOR_WHITE);
        }

        draw_button(0, 728, 1024, 40, "F2 - create a new file", COLOR_BLUE, COLOR_WHITE);

        draw_file_icons();

        if (show_crt_window == 1) {
            is_window_crt = 1;

            int wx = win_file_x; 
            int wy = win_file_y;

            draw_rect(wx + 4, wy + 4, 432, 260, COLOR_BLACK);
            draw_rect(wx,     wy,     432, 260, COLOR_WHITE);
            draw_rect(wx + 4, wy + 4, 424, 252, COLOR_LIGHT_GRAY);
            draw_rect(wx + 4, wy + 4, 424, 32, COLOR_BLACK);

            x = wx + 18;
            y = wy + 8;
            print("Create a new file", COLOR_WHITE);

            x = wx + 18;
            y = wy + 44;
            print("Name:", COLOR_WHITE);
            draw_rect(wx + 18, wy + 56, 404, 24, COLOR_WHITE);
            draw_rect(wx + 20, wy + 58, 400, 20, COLOR_BLACK);

            x = wx + 18;
            y = wy + 94;
            print("Content:", COLOR_WHITE);
            draw_rect(wx + 18, wy + 146, 404, 24, COLOR_WHITE); 
            draw_rect(wx + 20, wy + 148, 400, 20, COLOR_BLACK);
        }
    }
    else if (current_mode == 3) {
        draw_desktop();
        
        if (draw_0 == 1) {
            draw_button(78, 5, 136, 26, "Terminal", COLOR_GREEN, COLOR_WHITE);
            draw_button(191, 11, 15, 15, "x", COLOR_WHITE, COLOR_BLACK);
        }

        if (draw_1 == 1) {
            draw_button(238, 5, 136, 26, "Explorer", COLOR_RED, COLOR_WHITE);
            draw_button(351, 11, 15, 15, "x", COLOR_WHITE, COLOR_BLACK);
        }

        if (is_button_calc == 1) {
            draw_rect(10, 31, 72, 70, COLOR_LIGHT_GRAY);

            draw_button(10, 31, 70, 26, "Terminal", COLOR_GREEN, COLOR_WHITE);
            draw_button(10, 51, 70, 26, "Explorer", COLOR_RED, COLOR_WHITE);

            draw_button(10, 85, 15, 15, "x", COLOR_BLACK, COLOR_WHITE);
            draw_button(65, 85, 15, 15, "r", COLOR_BLACK, COLOR_WHITE);
        }

        x = 0;
        y = 96;
        print("System Information:\n", COLOR_WHITE);
        print("Battery Status: ", COLOR_WHITE);
        unsigned char battery_status = check_battery();
        if (battery_status) {
            print("OK\n", COLOR_WHITE);
        } else {
            print("BAD. Please insert a new CMOS battery\n", COLOR_WHITE);
        }
    }
    else {
        is_scaled = 0;

        if (draw_1 == 1) {
            draw_desktop();

            if (draw_0 == 1) {
                draw_button(78, 5, 136, 26, "Terminal", COLOR_GREEN, COLOR_WHITE);
                draw_button(191, 11, 15, 15, "x", COLOR_WHITE, COLOR_BLACK);
            }

            if (draw_1 == 1) {
                draw_button(238, 5, 136, 26, "Explorer", COLOR_RED, COLOR_WHITE);
                draw_button(351, 11, 15, 15, "x", COLOR_WHITE, COLOR_BLACK);
            }

            if (is_button_calc == 1) {
                draw_rect(10, 31, 72, 70, COLOR_LIGHT_GRAY);

                draw_button(10, 31, 70, 26, "Terminal", COLOR_GREEN, COLOR_WHITE);
                draw_button(10, 51, 70, 26, "Explorer", COLOR_RED, COLOR_WHITE);

                draw_button(10, 85, 15, 15, "x", COLOR_BLACK, COLOR_WHITE);
                draw_button(65, 85, 15, 15, "r", COLOR_BLACK, COLOR_WHITE);
            }

            if (is_button_files == 1) {
                draw_button(352, 250, 320, 36, "Files", COLOR_RED, COLOR_BLACK);
                draw_button(352, 350, 320, 36, "System", COLOR_GREEN, COLOR_BLACK);
            }
            else if (is_button_apps == 1) {
                draw_button(352, 250, 320, 36, "Files", COLOR_GREEN, COLOR_BLACK);
                draw_button(352, 350, 320, 36, "System", COLOR_RED, COLOR_BLACK);
            } 
        } else {
            current_mode = 0;
            ncount = 1;
        }
        is_button_calc = 0;
    }
    sti();
    #endif
}
