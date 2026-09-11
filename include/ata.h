#ifndef ATA_H
#define ATA_H
#include <stdint.h>

#define ATA_TIMEOUT 10000000
#define RAM_DISK_BASE 0x10000

extern volatile int ata_interrupt_received;

void ata_read_sector(uint32_t lba, uint16_t* buffer);
void ata_write_sector(uint32_t lba, uint16_t* buffer);
static void read_sector_ram(uint32_t lba, uint16_t* buffer);

#endif
