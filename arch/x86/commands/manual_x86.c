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

#include <video.h>
#include <utils.h>
#include <keyboard.h>
#include <stdint.h>
#include <pci.h>
#include <mm.h>
#include <coms.h>

void casm() {
    char *asm_name = strtok(NULL, " ");
    if (asm_name == NULL) {
        print("Usage: casm <asm_file>\n", COLOR_WHITE);
        return;
    }

    int fd = vfs_open(asm_name);
    if (fd < 0) {
        print("casm: File not found\n", COLOR_WHITE);
        return;
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

void empire() {
    play_imperial_march();
}

void cat() {
    char *file_name = strtok(NULL, " ");
    if (file_name == NULL) {
        print("Usage: cat <file_name>\n", COLOR_WHITE);
        return;
    }

    int fd = vfs_open(file_name);
    if (fd < 0) {
        print("cat: File not found\n", COLOR_WHITE);
        return;
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

void touch() {
    char *file_name = strtok(NULL, " ");
    if (file_name == NULL) {
        print("Usage: touch <file_name>\n", COLOR_WHITE);
        return;
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
}

void send() {
    char *packet_data = strtok(NULL, " ");
    if (packet_data == NULL) {
        print("Usage: send <message_text>\n", COLOR_WHITE);
        return;
    }

    uint32_t payload_size = strlen(packet_data); 
    uint8_t dest_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    
    send_pack((uint8_t*)packet_data, payload_size, dest_mac);
}

void forth() {
    char *forth_name = strtok(NULL, " ");
    if (forth_name == NULL) {
        print("Usage: forth <file_name>\n", COLOR_WHITE);
        return;
    }

    char name_11[11];
    format_fat_name(forth_name, name_11);

    read_file(name_11, content);
                    
    create_task(2);
}

void cube() {
    char *angle_y_str = strtok(NULL, " ");
    char *angle_x_str = strtok(NULL, " ");

    int angle_y = 0;
    int angle_x = 0;
    
    if (angle_y_str != NULL) {
        angle_y = atoi(angle_y_str);
    }
    if (angle_x_str != NULL) {
        angle_x = atoi(angle_x_str);
    }
    draw_cube(angle_y, angle_x);
}
