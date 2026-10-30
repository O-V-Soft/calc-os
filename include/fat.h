#ifndef FAT_H
#define FAT_H
#include <stdint.h>

struct fat12_bpb {
    uint8_t  jmp[3];           
    char     oem_id[8];        
    uint16_t bytes_per_sector; 
    uint8_t  sectors_per_cluster; 
    uint16_t reserved_sectors;  
    uint8_t  num_fats;        
    uint16_t root_entries;    
    uint16_t total_sectors_short;
    uint8_t  media_type;
    uint16_t fat_size_sectors;  
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_long;
} __attribute__((packed));

struct fat12_entry {
    char        name[8];
    char        ext[3];
    uint8_t     attributes;
    uint8_t     reserved;
    uint8_t     creation_time_ms;
    uint16_t    creation_time;
    uint16_t    creation_date;
    uint16_t    last_access_date;
    uint16_t    ignore_in_fat12; 
    uint16_t    last_write_time;
    uint16_t    last_write_date;
    uint16_t    first_cluster;   
    uint32_t    file_size;
} __attribute__((packed));

typedef struct {
    uint16_t cluster;
    uint32_t offset;
    uint8_t used;
} fat12_fd_t;

void read_file(const char* filename_11, uint8_t* buffer);
void create_file(const char* name_11, uint8_t* data, int size);
void list_files();
uint16_t find_file_in_root(const char* target_name_11);
uint16_t get_fat_entry(uint16_t cluster_n, uint8_t* fat_table);
void set_fat_entry(uint16_t cluster_n, uint16_t value, uint8_t* fat_table);

int fat12_read(int fd, void* buf, size_t count);
int fat12_create(const char* path, const uint8_t* data, size_t size);

extern int disk_initialized;
extern uint8_t cached_fat[512 * 9];
extern struct fat12_bpb cached_bpb;
extern struct fs_driver fat12_driver;
extern fat12_fd_t fat12_fd_table[10];

#endif
