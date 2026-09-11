#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <ata.h>
#include <idt.h>
#include <stdint.h>
#include <fat.h>

static uint8_t use_ram = 0;

#define ATA_REG_SECTOR_COUNT 0x1F2
#define ATA_REG_LBA_LOW      0x1F3

void ata_handler() {
    ata_interrupt_received = 1;

    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

int detect_ata() {
    outb(ATA_REG_SECTOR_COUNT, 0x55);
    outb(ATA_REG_LBA_LOW, 0xAA); 
    
    if (inb(ATA_REG_SECTOR_COUNT) != 0x55) {
        return 0;
    }

    outb(ATA_REG_SECTOR_COUNT, 0xAA);
    outb(ATA_REG_LBA_LOW, 0x55);
    
    if (inb(ATA_REG_SECTOR_COUNT) != 0xAA) {
        return 0; 
    }

    return 1; 
}

static int ata_wait_bsy() {
    uint32_t timeout = ATA_TIMEOUT;
    while (inb(0x1F7) & 0x80) {
        if (--timeout == 0) {
            use_ram = 1; 
            return -1;
        }
        __asm__ __volatile__("pause");
    }
    return 0;
}

static int ata_wait_drq() {
    uint32_t timeout = ATA_TIMEOUT;
    while (!(inb(0x1F7) & 0x08)) {
        if (--timeout == 0) {
            use_ram = 1;
            return -1;
        }
        __asm__ __volatile__("pause");
    }
    return 0;
}

static void read_sector_ram(uint32_t lba, uint16_t* buffer) {
    uint32_t offset = lba * 512;
    volatile uint16_t* src = (volatile uint16_t*)(uintptr_t)(RAM_DISK_BASE + offset);

    for (int i = 0; i < 256; i++) {
        buffer[i] = src[i];
    }
}

static void write_sector_ram(uint32_t lba, uint16_t* buffer) {
    uint32_t offset = lba * 512;
    volatile uint16_t* dest = (volatile uint16_t*)(uintptr_t)(RAM_DISK_BASE + offset);

    for (int i = 0; i < 256; i++) {
        dest[i] = buffer[i];
    }
}

void ata_read_sector(uint32_t lba, uint16_t* buffer) {
    static int check_done = 0;
    
    if (!check_done) {
        if (detect_ata() == 0) {
            use_ram = 1; 
        } else {
            use_ram = 0; 
        }
        check_done = 1;
    }

    if (use_ram == 1) {
        read_sector_ram(lba, buffer);
        return;
    }

    asm volatile("sti");

    if (ata_wait_bsy() != 0) {
        read_sector_ram(lba, buffer);
        return;
    }

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1); 
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));

    ata_interrupt_received = 0;

    outb(0x1F7, 0x20); 

    uint32_t timeout = ATA_TIMEOUT;
    while (!ata_interrupt_received) {
        if (--timeout == 0) {
            use_ram = 1; 
            read_sector_ram(lba, buffer);
            return;
        }
        __asm__ __volatile__("pause");
    }

    for (int i = 0; i < 256; i++) {
        buffer[i] = inw(0x1F0); 
    }
}

void ata_write_sector(uint32_t lba, uint16_t* buffer) {
    static int check_done = 0;
    
    if (!check_done) {
        if (detect_ata() == 0) {
            use_ram = 1; 
        } else {
            use_ram = 0; 
        }
        check_done = 1;
    }

    if (use_ram == 1) {
        write_sector_ram(lba, buffer);
        return;
    }

    asm volatile("sti");

    if (ata_wait_bsy() != 0) {
        return; 
    }

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));

    outb(0x1F7, 0x30); 

    if (ata_wait_bsy() != 0 || ata_wait_drq() != 0) {
        return;
    }

    for (int i = 0; i < 256; i++) {
        outw(0x1F0, buffer[i]); 
    }

    ata_interrupt_received = 0;
    
    uint32_t timeout = ATA_TIMEOUT;
    while (!ata_interrupt_received) {
        if (--timeout == 0) {
            use_ram = 1;
            return;
        }
        __asm__ __volatile__("pause");
    }
}
