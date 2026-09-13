#if !defined(__riscv)
#include <fat.h>
#include <cmos.h>
#include <mouse.h>
#include <idt.h>
#include <ata.h>
#include <sound.h>
#include <forth.h>
#include <task.h>
#include <vfs.h>
#include <casm.h>
#endif

#include <video.h>
#include <utils.h>
#include <keyboard.h>
#include <stdint.h>
#include <pci.h>
#include <mm.h>
#include <coms.h>

#if defined(__riscv)
#include <riscv.h>
#endif

char command[256];
char name[128];
char content[512];
char byte_str[16];

int current_uid = 1; 
const char* root_password = "102030!";

int win_file_x = 296;
int win_file_y = 244;

void system() {
refresh:
    ncount = 0;

    if (is_crushed == 1) {
        #if !defined(__riscv)
        delay_ticks(300);
        is_scaled = 0;
        is_crushed = 0;
        #endif
    }

    draw_rect(0, 0, 1024, 40, COLOR_LIGHT_GRAY);

    draw_button(10, 5, 56, 26, "CalcOS", COLOR_BLACK, COLOR_WHITE);

    sti();

    graphics();

    while (1) {
        if (ncount == 1) goto refresh;

        #if !defined(__riscv)
        update_system();
        #endif

        if (ncount == 1) goto refresh;

        if (current_mode == 0) {
            is_scaled = 2;
            if (current_uid == 0) {
                print("# ", COLOR_WHITE);
            } else {
                print("$ ", COLOR_WHITE);
            }
            
            input_wait_string(command);

            if (ncount == 1) goto refresh;

            print("\n", COLOR_WHITE);

            char *command2 = strtok(command, " ");

            if (command2 != NULL) {
                int result = execute_command(command);
                if (result == 0) {
                    continue;
                }
                else {
                    if (command[0] != '\0') {
                        char cmd_with_ext[16];
                        char cmd_file_fat[12];
                        
                        int c_len = 0;
                        while (command[c_len] != '\0' && c_len < 8) {
                            char c = command[c_len];
                            if (c >= 'a' && c <= 'z') {
                                c = c - 'a' + 'A';
                            }
                            cmd_with_ext[c_len] = c;
                            c_len++;
                        }
                        
                        cmd_with_ext[c_len++] = '.';
                        cmd_with_ext[c_len++] = 'B';
                        cmd_with_ext[c_len++] = 'I';
                        cmd_with_ext[c_len++] = 'N';
                        cmd_with_ext[c_len] = '\0';

                        format_fat_name(cmd_with_ext, cmd_file_fat);
                        cmd_file_fat[11] = '\0'; 

                        uint16_t start_cluster = find_file_in_root(cmd_file_fat);
                        
                        if (start_cluster != 0) {
                            sys_exec(cmd_file_fat);
                        } else {
                            print("Unknown command. Type 'help'\n", COLOR_WHITE);
                        }
                    }
                }
            }
        }
        else if (current_mode == 2) {
            int code = get_scancode();
            if (code != 0) {
                handle_hotkeys(code);
                if (code == 0x3C) {
                    show_crt_window = 1;
                    ncount = 1;
                }
            }

            if (ncount == 1) goto refresh;

            if (show_crt_window == 1) {
                is_window_crt = 1;
                name_clear();
                content_clear();

                if (ncount == 1) goto refresh;

                int wx = win_file_x; 
                int wy = win_file_y;

                x = wx + 26;
                y = wy + 60;
                input_wait_string(name);

                x = wx + 26;
                y = wy + 150;
                input_wait_string(content);

                if (ncount == 1) goto refresh;

                print("\n", COLOR_WHITE);

                int len = 0;
                while (content[len] != '\0') len++;
                if (len > 512) len = 512;

                char name_11[11];
                format_fat_name(name, name_11);

                uint8_t buffer[512] = {0};
                for (int j = 0; j < len; j++) buffer[j] = (uint8_t)content[j];

                create_file(name_11, buffer, len);

                show_crt_window = 0;
                is_window_crt = 0;
                ncount = 1;
            }
        }
        else if (current_mode == 5) {
            int code = get_scancode();
            if (code != 0) {
                handle_hotkeys(code);
            }

            if (ncount == 1) goto refresh;
        }
        else {
            int code = get_scancode();
            if (code != 0) handle_hotkeys(code);
            if (ncount == 1) goto refresh;
        }
    }
}

void boot() {
    is_scaled = 1;
    screen_clear();

    print("Scanning PCI...                              ", COLOR_WHITE);
    pci_scan();
    print("[OK]\n\n", COLOR_WHITE);

    print("Initializing memory manager...               ", COLOR_WHITE);
    init_memory_manager();
    print("[OK]\n\n", COLOR_WHITE);

    print("Initializing ethernet card...                ", COLOR_WHITE);
    int is_rtl8139_found = rtl8139_find();
    if (is_rtl8139_found) {
        rtl8139_init();
        print("[OK]\n\n", COLOR_WHITE);
    } else {
        print("[ERR]\n\n", COLOR_WHITE);
    }

    print("Mounting FAT12 filesystem...                 ", COLOR_WHITE);
    vfs_mount("/", &fat12_driver);
    print("[OK]\n", COLOR_WHITE);

    delay_ticks(100);

    screen_clear();
    init_palette();
    draw_rect(0, 0, 1024, 768, COLOR_WHITE);

    x = 0;
    y = 10;

    is_scaled = 1;
    print("Welcome to CalcOS!", COLOR_BLACK);
    is_scaled = 2;
    play_startup_sound();

    is_scaled = 0;
    __asm__ __volatile__("sti");
    *(volatile char*)__rodata_start = 'X';
}
