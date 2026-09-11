#ifndef RISCV_H
#define RISCV_H
#include <stdint.h>

extern volatile uint8_t* uart_reg_base;

#define UART_REG(reg) (uart_reg_base + (reg))
#define UART_RBR (volatile uint8_t *)(UART_BASE + 0) 
#define UART_LSR (volatile uint8_t *)(UART_BASE + 5)
#define UART_LSR_DR 0x01
#define UART_BASE 0x10000000

void uart_put_char(char c);
void uart_printk(const char* str);
char uart_get_char(void);
void get_string(char *buffer);

static inline uint32_t read_csr(const char* reg) {
    uintptr_t value;
    asm volatile("csrr %0, " : "=r"(value) : "i"(reg)); 
    return value;
}

#define read_mstatus() ({ uint32_t __v; asm volatile("csrr %0, mstatus" : "=r"(__v)); __v; })
#define read_mie()     ({ uint32_t __v; asm volatile("csrr %0, mie"     : "=r"(__v)); __v; })
#define read_mhartid() ({ uint32_t __v; asm volatile("csrr %0, mhartid" : "=r"(__v)); __v; })

#define NULL ((void*)0)

void pci_print_devices();
void pci_scan();
uint32_t pci_read_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_write_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
uint16_t pci_read_config_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_write_config_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t value);
uint8_t pci_read_config_byte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint32_t pci_find_device(uint16_t vendor_id, uint16_t device_id, uint8_t bar_offset);

#define RISCV_ECAM_BASE 0x30000000

#define RISCV_CLINT_MTIME    ((volatile uint64_t*)0x0200BFF8)
#define RISCV_CLINT_MTIMECMP ((volatile uint64_t*)0x02004000)

#define TIMER_FREQUENCY_HZ   10000000

extern unsigned int timer_ticks;

void init_timer();
extern void trap_vector();

#define PCI_VENDOR_TABLE_SIZE (sizeof(pci_vendor_table) / sizeof(pci_vendor_table[0]))

#define cli() __asm__ __volatile__ ("nop");
#define sti() __asm__ __volatile__ ("nop");

#endif
