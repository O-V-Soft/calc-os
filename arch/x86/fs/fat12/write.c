#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <stdint.h>
#include <ata.h>
#include <fat.h>

static uint16_t bpb_buf[256];
static uint8_t fat_table[512 * 9];
static uint16_t root_buf[256];

void create_file(const char* name_11, uint8_t* data, int size) {
    ata_read_sector(0, (uint16_t*)bpb_buf);
    struct fat12_bpb* bpb = (struct fat12_bpb*)bpb_buf;
    for (int i = 0; i < bpb->fat_size_sectors; i++)
        ata_read_sector(bpb->reserved_sectors + i, (uint16_t*)(fat_table + (i * 512)));

    uint16_t free_cluster = 0;
    for (uint16_t i = 2; i < 4084; i++) {
        if (get_fat_entry(i, fat_table) == 0) {
            free_cluster = i;
            break;
        }
    }
    if (free_cluster == 0) return; 

    uint32_t root_lba = bpb->reserved_sectors + (bpb->num_fats * bpb->fat_size_sectors);
    uint32_t root_sectors = ((bpb->root_entries * 32) + 511) / 512;
    
    int found_entry = 0;
    for (uint32_t s = 0; s < root_sectors; s++) {
        ata_read_sector(root_lba + s, (uint16_t*)root_buf);
        struct fat12_entry* entries = (struct fat12_entry*)root_buf;

        for (int i = 0; i < 16; i++) {
            if (entries[i].name[0] == 0x00 || (uint8_t)entries[i].name[0] == 0xE5) {
                memcpy(entries[i].name, name_11, 11);
                entries[i].attributes = 0x20; 
                entries[i].first_cluster = free_cluster;
                entries[i].file_size = size; 

                ata_write_sector(root_lba + s, (uint16_t*)root_buf);
                found_entry = 1;
                break;
            }
        }
        if (found_entry) break;
    }

    set_fat_entry(free_cluster, 0xFFF, fat_table);
    for (int i = 0; i < bpb->fat_size_sectors; i++)
        ata_write_sector(bpb->reserved_sectors + i, (uint16_t*)(fat_table + (i * 512)));

    uint32_t data_lba = root_lba + root_sectors;

    uint8_t write_buf[512];
    for (int i = 0; i < 512; i++) write_buf[i] = 0;

    int to_copy = (size > 512) ? 512 : size;
    for (int i = 0; i < to_copy; i++) {
        write_buf[i] = data[i];
    }

    ata_write_sector(data_lba + (free_cluster - 2), (uint16_t*)write_buf);

    if (disk_initialized) {
        disk_initialized = 0;
    }
}
