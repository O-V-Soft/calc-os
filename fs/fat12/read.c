#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <stdint.h>
#include <ata.h>
#include <fat.h>
#include <pci.h>
#include <vfs.h>

void read_file_basic(uint16_t cluster, uint8_t* destination, uint8_t* fat_table, struct fat12_bpb* bpb) {
    uint32_t root_dir_sectors = ((bpb->root_entries * 32) + (bpb->bytes_per_sector - 1)) / bpb->bytes_per_sector;
    uint32_t fat_size = bpb->num_fats * bpb->fat_size_sectors;
    uint32_t data_lba = bpb->reserved_sectors + fat_size + root_dir_sectors;

    uint16_t current_cluster = cluster;

    while (current_cluster < 0xFF8) {
        uint32_t cluster_lba = ((current_cluster - 2) * bpb->sectors_per_cluster) + data_lba;

        for (uint8_t i = 0; i < bpb->sectors_per_cluster; i++) {
            ata_read_sector(cluster_lba + i, (uint16_t*)destination);
            destination = destination + 512;
        }

        current_cluster = get_fat_entry(current_cluster, fat_table);
        
        if (current_cluster == 0) break;
    }
}

void read_file(const char* filename_11, uint8_t* buffer) {
    if (!disk_initialized) {
        uint16_t bpb_buf[256];
        ata_read_sector(0, (uint16_t*)bpb_buf);
        cached_bpb = *(struct fat12_bpb*)bpb_buf;

        for (uint32_t i = 0; i < cached_bpb.fat_size_sectors; i++) {
            ata_read_sector(cached_bpb.reserved_sectors + i, 
                            (uint16_t*)(cached_fat + (i * 512)));
        }
        disk_initialized = 1;
    }

    uint16_t start_cluster = find_file_in_root(filename_11);

    if (start_cluster != 0) {
        read_file_basic(start_cluster, buffer, cached_fat, &cached_bpb);
    }
}

void list_files() {
    static uint8_t sector_buffer[512];
    struct fat12_bpb bpb;

    ata_read_sector(0, (uint16_t*)sector_buffer);
    memcpy(&bpb, sector_buffer, sizeof(struct fat12_bpb));

    uint32_t root_lba = bpb.reserved_sectors + (bpb.num_fats * bpb.fat_size_sectors);
    uint32_t root_sectors = ((bpb.root_entries * 32) + (bpb.bytes_per_sector - 1)) / bpb.bytes_per_sector;

    for (uint32_t s = 0; s < root_sectors; s++) {
        ata_read_sector(root_lba + s, (uint16_t*)sector_buffer);
        struct fat12_entry* entries = (struct fat12_entry*)sector_buffer;

        int entries_per_sector = bpb.bytes_per_sector / sizeof(struct fat12_entry);
        for (int i = 0; i < entries_per_sector; i++) {
            if (entries[i].name[0] == 0x00) break;
            if ((uint8_t)entries[i].name[0] == 0xE5) continue;

            char name_buf[9];
            char ext_buf[4];
            int name_len = 0;
            int ext_len = 0;

            for (int j = 0; j < 8; j++) {
                if (entries[i].name[j] != ' ') {
                    name_buf[name_len++] = entries[i].name[j];
                }
            }

            name_buf[name_len] = '\0';

            for (int j = 0; j < 3; j++) {
                if (entries[i].ext[j] != ' ') {
                    ext_buf[ext_len++] = entries[i].ext[j];
                }
            }
            
            ext_buf[ext_len] = '\0';

            printk(name_buf, COLOR_WHITE);

            if(ext_len > 0) {
                printk(".", COLOR_WHITE);
                printk(ext_buf, COLOR_WHITE);
            }

            printk("\n", COLOR_WHITE);
        }
    }
}

void draw_file_icons() {
    uint16_t buffer[256];
    ata_read_sector(0, buffer);
    struct fat12_bpb* bpb = (struct fat12_bpb*)buffer;

    uint32_t root_lba = bpb->reserved_sectors + (bpb->num_fats * bpb->fat_size_sectors);
    uint32_t root_sectors = ((bpb->root_entries * 32) + (bpb->bytes_per_sector - 1)) / bpb->bytes_per_sector;

    int icon_index = 0;
    for (uint32_t s = 0; s < root_sectors; s++) {
        ata_read_sector(root_lba + s, buffer);
        struct fat12_entry* entries = (struct fat12_entry*)buffer;

        for (int i = 0; i < 16; i++) {
            if (entries[i].name[0] == 0x00) return;
            if ((uint8_t)entries[i].name[0] == 0xE5) continue;
            if (entries[i].attributes == 0x0F) continue;

            int col = icon_index % 5;
            int row = icon_index / 5;
            int icon_x = 20 + col * 180;
            int icon_y = 100 + row * 100;

            draw_rounded_rect(icon_x, icon_y, 130, 30, 4, COLOR_LIGHT_GRAY);

            int center_x = icon_x + 20; 
            int center_y = icon_y + 15;

            draw_cube(center_x, center_y, 200, 15, 15);

            char name_buf[9];
            char ext_buf[4];
            int name_len = 0;
            int ext_len = 0;

            for (int j = 0; j < 8; j++) {
                if (entries[i].name[j] != ' ') {
                    name_buf[name_len++] = entries[i].name[j];
                }
            }
            name_buf[name_len] = '\0';

            for (int j = 0; j < 3; j++) {
                if (entries[i].ext[j] != ' ') {
                    ext_buf[ext_len++] = entries[i].ext[j];
                }
            }
            ext_buf[ext_len] = '\0';

            x = icon_x + 40;
            y = icon_y + 8;
            print(name_buf, COLOR_WHITE);
            if (ext_len > 0) {
                print(".", COLOR_WHITE);
                print(ext_buf, COLOR_WHITE);
            }

            icon_index++;
        }
    }
}
