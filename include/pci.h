#ifndef PCI_H
#define PCI_H
#include <stdint.h>

#define NULL ((void*)0)

typedef struct {
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_id;   
    uint8_t subclass; 
    uint8_t prog_if;   
} pci_device_t;

void pci_print_devices();
void pci_scan();
uint32_t pci_read_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_write_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
uint16_t pci_read_config_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_write_config_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t value);
uint8_t pci_read_config_byte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint32_t pci_find_device(uint16_t vendor_id, uint16_t device_id, uint8_t bar_offset);

#if !defined(__riscv)
void rtl8139_init();
void send_pack(uint8_t* payload, uint16_t payload_size, uint8_t dest_mac[6]);
uint8_t read_pack();
int rtl8139_find();
void graphics_init();
#endif

extern pci_device_t devices[32];
extern int device_count;

#define PCI_VENDOR_TABLE_SIZE (sizeof(pci_vendor_table) / sizeof(pci_vendor_table[0]))

#endif
