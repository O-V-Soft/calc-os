#include <stdint.h>
#include <mm.h>

__attribute__((section(".bss.safe"), aligned(4096))) uint32_t page_directory[1024];
__attribute__((section(".bss.safe"), aligned(4096))) uint32_t first_page_table[1024];

void init_paging() {
    for (uint32_t i = 0; i < 1024; i++) {
        page_directory[i] = (i * 0x400000) | 0x83;
    }

    for (uint32_t j = 0; j < 1024; j++) {
        uint32_t phys_addr = j * 4096;

        first_page_table[j] = phys_addr | 0x03;
    }

    uint32_t table_addr = (uint32_t)first_page_table;
    page_directory[0] = (table_addr & 0xFFFFF000) | 0x03;
}

void enable_paging() {
    uint32_t pd_address = (uint32_t)&page_directory;

    __asm__ __volatile__ (
        "cli\n\t"

        "mov %%cr4, %%eax\n\t"
        "or $0x10, %%eax\n\t"
        "mov %%eax, %%cr4\n\t"

        "mov %0, %%eax\n\t"
        "mov %%eax, %%cr3\n\t"

        "mov %%cr0, %%eax\n\t"
        "or $0x80010000, %%eax\n\t"
        "mov %%eax, %%cr0\n\t"

        "jmp 1f\n\t"
        "1:\n\t"
        :
        : "r"(pd_address)
        : "eax", "memory"
    );
}