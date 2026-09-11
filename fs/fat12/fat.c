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

typedef struct {
    uint16_t cluster;
    uint32_t offset;
    uint8_t used;
} fat12_fd_t;

static fat12_fd_t fat12_fd_table[10];

uint16_t get_fat_entry(uint16_t cluster_n, uint8_t* fat_table) {
    uint32_t offset = cluster_n + (cluster_n / 2);
    
    uint16_t value = *(uint16_t*)(fat_table + offset);

    if (cluster_n % 2 == 0) {
        return value & 0x0FFF;
    } else {
        return value >> 4;
    }
}

void load_root_directory() {
    uint16_t buffer[256]; 

    ata_read_sector(0, buffer);
    struct fat12_bpb* bpb = (struct fat12_bpb*)buffer;

    uint32_t root_dir_lba = bpb->reserved_sectors + (bpb->num_fats * bpb->fat_size_sectors);
    uint32_t root_dir_sectors = ((bpb->root_entries * 32) + (bpb->bytes_per_sector - 1)) / bpb->bytes_per_sector;

    ata_read_sector(root_dir_lba, buffer);

    struct fat12_entry* entries = (struct fat12_entry*)buffer;

    for (int i = 0; i < 16; i++) {
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
        if (ext_len > 0) {
            printk(".", COLOR_WHITE);
            printk(ext_buf, COLOR_WHITE);
        }
        printk("\n", COLOR_BLACK); 
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

void read_cluster(uint16_t cluster, uint16_t* target_buffer, uint32_t data_region_lba, uint8_t sectors_per_cluster) {
    uint32_t lba = ((cluster - 2) * sectors_per_cluster) + data_region_lba;
    
    for (int i = 0; i < sectors_per_cluster; i++) {
        ata_read_sector(lba + i, target_buffer + (i * 256)); 
    }
}

uint16_t find_free_cluster(uint8_t* fat_table) {
    for (uint16_t i = 2; i < 4084; i++) {
        if (get_fat_entry(i, fat_table) == 0x000) {
            return i;
        }
    }
    return 0; 
}

void set_fat_entry(uint16_t cluster_n, uint16_t value, uint8_t* fat_table) {
    uint32_t offset = cluster_n + (cluster_n / 2);
    uint16_t* ptr = (uint16_t*)(fat_table + offset);

    if (cluster_n % 2 == 0) {
        *ptr = (*ptr & 0xF000) | (value & 0x0FFF);
    } else {
        *ptr = (*ptr & 0x000F) | (value << 4);
    }
}

static int fat12_open(const char* path) {
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

static int fat12_read(int fd, void* buf, size_t count) {
    if (fd < 3 || fd >= 10 || !fat12_fd_table[fd].used)
        return -1;

    uint16_t cluster = fat12_fd_table[fd].cluster;
    uint32_t offset  = fat12_fd_table[fd].offset;

    static uint8_t sector_buf[512];

    uint32_t root_lba = cached_bpb.reserved_sectors +
                        (cached_bpb.num_fats * cached_bpb.fat_size_sectors);

    uint32_t root_sectors = ((cached_bpb.root_entries * 32) + 511) / 512;

    uint32_t data_lba = root_lba + root_sectors;

    ata_read_sector(data_lba + (cluster - 2), (uint16_t*)sector_buf);

    size_t bytes_to_copy = count;
    if (offset + bytes_to_copy > 512)
        bytes_to_copy = 512 - offset;

    memcpy(buf, sector_buf + offset, bytes_to_copy);

    fat12_fd_table[fd].offset += bytes_to_copy;

    return bytes_to_copy;
}

static int fat12_close(int fd) {
    if (fd < 3 || fd >= 10 || !fat12_fd_table[fd].used)
        return -1;

    fat12_fd_table[fd].used = 0;
    fat12_fd_table[fd].cluster = 0;
    fat12_fd_table[fd].offset = 0;
    return 0;
}

static int fat12_list() {
    list_mounts();
    return 0;
}

static int list_basic() {
    list_files();
    return 0;
}

static int fat12_create(const char* path, const uint8_t* data, size_t size) {
    char name11[11];
    format_fat_name(path, name11);

    create_file(name11, (uint8_t*)data, (int)size);
    return 0;
}

static int fat12_umount() {
    disk_initialized = 0; 
    
    for (int i = 0; i < 10; i++) {
        fat12_fd_table[i].used = 0;
        fat12_fd_table[i].cluster = 0;
        fat12_fd_table[i].offset = 0;
    }
    return 0;
}

struct fs_driver fat12_driver = {
    .open = fat12_open,
    .read = fat12_read,
    .close = fat12_close,
    .list = fat12_list,
    .list_basic = list_basic,
    .create = fat12_create,
    .umount = fat12_umount
};
