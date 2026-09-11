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

    cli();

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
                if (compare_strings(command, "help")) {
                    help();
                }
                else if (compare_strings(command, "su")) {
                   su();
                }
                else if (compare_strings(command, "exit")) {
                   exit2(); 
                }
                else if (compare_strings(command, "casm")) {
                    char *asm_name = strtok(NULL, " ");
                    if (asm_name == NULL) {
                        print("Usage: casm <asm_file>\n", COLOR_WHITE);
                        continue;
                    }

                    int fd = vfs_open(asm_name);
                    if (fd < 0) {
                        print("casm: File not found\n", COLOR_WHITE);
                        continue;
                    }

                    uint8_t buffer[512];
                    int n = vfs_read(fd, buffer, 512);
                    vfs_close(fd);

                    name_clear();
                    print("Out file name: ", COLOR_WHITE);
                    input_wait_string(name);
                    print("\n", COLOR_WHITE);
                    assemble_to_bin(name, buffer);
                }
                else if (compare_strings(command, "empire")) {
                    play_imperial_march();
                }
                else if (compare_strings(command, "cat")) {
                    char *file_name = strtok(NULL, " ");
                    if (file_name == NULL) {
                        print("Usage: cat <file_name>\n", COLOR_WHITE);
                        continue;
                    }

                    int fd = vfs_open(file_name);
                    if (fd < 0) {
                        print("cat: File not found\n", COLOR_WHITE);
                        continue;
                    }

                    uint8_t buffer[512];
                    int n = vfs_read(fd, buffer, 511);
                    vfs_close(fd);

                    if (n > 0) {
                        buffer[n] = '\0';
                        for (int i = 0; i < n; i++) {
                            char c = buffer[i];
                            if (c == '\0' || (uint8_t)c == 0xE5 || (uint8_t)c == 0xFF) {
                                break;
                            }
                            put_char(c, COLOR_WHITE);
                        }
                    } else {
                        print("cat: Empty or unreadable file\n", COLOR_WHITE);
                    }
                    print("\n", COLOR_WHITE);
                }
                else if (compare_strings(command, "cln")) {
                    cln();
                }
                else if (compare_strings(command, "touch")) {
                    char *file_name = strtok(NULL, " ");
                    if (file_name == NULL) {
                        print("Usage: touch <file_name>\n", COLOR_WHITE);
                        continue;
                    }

                    is_scaled = 2;
                    content_clear();
                    screen_clear();

                    draw_rect(0, 0, 1024, 30, COLOR_LIGHT_GRAY); 
                    x = 10; y = 8;
                    print("Editing: ", COLOR_BLACK);
                    print(file_name, COLOR_BLACK); 

                    draw_rect(0, 738, 1024, 30, COLOR_LIGHT_GRAY); 
                    x = 10; y = 746;
                    print("ESC: Save and Exit", COLOR_BLACK);

                    x = 0; y = 40;

                    uint8_t file_buffer[512];
                    memset(file_buffer, 0, sizeof(file_buffer));

                    input_wait_multiline((char *)file_buffer);

                    int buffer_ptr = 0;
                    while (file_buffer[buffer_ptr] != '\0' && buffer_ptr < 512) {
                        buffer_ptr++;
                    }

                    vfs_create(file_name, file_buffer, buffer_ptr);
                    screen_clear();
                    goto refresh;
                }
                else if (compare_strings(command, "send")) {
                    char *packet_data = strtok(NULL, " ");
                    if (packet_data == NULL) {
                        print("Usage: send <message_text>\n", COLOR_WHITE);
                        continue;
                    }

                    uint32_t payload_size = strlen(packet_data); 
                    uint8_t dest_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    
                    send_pack((uint8_t*)packet_data, payload_size, dest_mac);
                }
                else if (compare_strings(command, "forth")) {
                    char *forth_name = strtok(NULL, " ");
                    if (forth_name == NULL) {
                        print("Usage: forth <file_name>\n", COLOR_WHITE);
                        continue;
                    }

                    char name_11[11];
                    format_fat_name(forth_name, name_11);

                    read_file(name_11, content);
                    
                    create_task(2);
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
    vfs_umount("/");
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
}
