#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <stdint.h>
#include <ata.h>
#include <fat.h>
#include <vfs.h>

int disk_initialized = 0;
uint8_t cached_fat[512 * 9];
struct fat12_bpb cached_bpb;

fat12_fd_t fat12_fd_table[10];

uint16_t get_fat_entry(uint16_t cluster_n, uint8_t* fat_table) {
    uint32_t offset = cluster_n + (cluster_n / 2);
    
    uint16_t value = *(uint16_t*)(fat_table + offset);

    if (cluster_n % 2 == 0) {
        return value & 0x0FFF;
    } else {
        return value >> 4;
    }
}

uint16_t find_file_in_root(const char* target_name_11) {
    uint16_t buffer[256]; 
    
    ata_read_sector(0, buffer);
    struct fat12_bpb* bpb = (struct fat12_bpb*)buffer;

    uint32_t root_lba = bpb->reserved_sectors + (bpb->num_fats * bpb->fat_size_sectors);

    if (bpb->bytes_per_sector == 0) {
        printk("FATAL: RAM disk memory is empty or corrupted.\n", COLOR_WHITE);
        return 0; 
    }

    uint32_t root_sectors = ((bpb->root_entries * 32) + (bpb->bytes_per_sector - 1)) / bpb->bytes_per_sector;

    for (uint32_t s = 0; s < root_sectors; s++) {
        ata_read_sector(root_lba + s, buffer);
        struct fat12_entry* entries = (struct fat12_entry*)buffer;

        for (int i = 0; i < 16; i++) {
            if (entries[i].name[0] == 0x00) return 0;
            
            if ((uint8_t)entries[i].name[0] == 0xE5) continue;

            if (memcmp(entries[i].name, target_name_11, 11)) {
                return entries[i].first_cluster;
            }
        }
    }

    return 0; 
}

uint16_t find_free_cluster(uint8_t* fat_table) {
    for (uint16_t i = 2; i < 4084; i++) {
        if (get_fat_entry(i, fat_table) == 0x000) {
            return i;
        }
    }
    return 0; 
}

int fat12_open(const char* path) {
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

    char name11[11];
    format_fat_name(path, name11);  

    uint16_t cluster = find_file_in_root(name11);
    if (cluster == 0)
        return -1;

    for (int i = 3; i < 10; i++) {
        if (!fat12_fd_table[i].used) {
            fat12_fd_table[i].used = 1;
            fat12_fd_table[i].cluster = cluster;
            fat12_fd_table[i].offset = 0;
            return i;
        }
    }

    return -1;
}

int fat12_close(int fd) {
    if (fd < 3 || fd >= 10 || !fat12_fd_table[fd].used)
        return -1;

    fat12_fd_table[fd].used = 0;
    fat12_fd_table[fd].cluster = 0;
    fat12_fd_table[fd].offset = 0;
    return 0;
}

int fat12_list() {
    list_files();
    return 0;
}

struct fs_driver fat12_driver = {
    .open = fat12_open,
    .read = fat12_read,
    .close = fat12_close,
    .list = fat12_list,
    .create = fat12_create
};
