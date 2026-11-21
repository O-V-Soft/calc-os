#include <fat.h>
#include <cmos.h>
#include <mouse.h>
#include <idt.h>
#include <ata.h>
#include <sound.h>
#include <task.h>
#include <vfs.h>
#include <video.h>
#include <utils.h>
#include <keyboard.h>
#include <stdint.h>
#include <pci.h>
#include <mm.h>
#include <net_dev.h>
#include <net.h>
#include <acpi.h>

extern uint32_t __bss_start[];
extern uint32_t __bss_safe_end[];

static void clear_bss() {
    for (uint32_t *address = __bss_start; address < __bss_safe_end; address++) {
        *address = 0;
    }
}

void boot() {
    is_scaled = 1;
    screen_clear();

    printk("Scanning PCI...                              ", COLOR_WHITE);
    pci_scan();
    printk("[OK]\n\n", COLOR_WHITE);

    printk("Initializing memory manager...               ", COLOR_WHITE);
    init_memory_manager();
    printk("[OK]\n\n", COLOR_WHITE);

    printk("Initializing ACPI...                         ", COLOR_WHITE);
    acpi_init();
    printk("[OK]\n\n", COLOR_WHITE);

    printk("Initializing FAT12 driver...                 ", COLOR_WHITE);
    vfs_register_driver(&fat12_driver);
    printk("[OK]\n\n", COLOR_WHITE);
	
    printk("Initializing ethernet card...                ", COLOR_WHITE);
    int is_rtl8139_found = rtl8139_find();
    if (is_rtl8139_found >= 0) {
        rtl8139_init();
	    rtl8139_register();
        struct net_device* net_device = get_primary_net_device();
        printk("[OK]\n\n", COLOR_WHITE);
    } else {
        printk("[ERR]\n\n", COLOR_WHITE);
    }

    printk("Initializing icon...                         ", COLOR_WHITE);
    icon_init();
    printk("[OK]\n\n", COLOR_WHITE);

    delay_ticks(100);

    screen_clear();
    init_palette();
    draw_rect(0, 0, 1024, 768, COLOR_WHITE);

    x = 0;
    y = 10;

    is_scaled = 1;
    printk("Welcome to CalcOS!", COLOR_BLACK);
    is_scaled = 2;
    play_startup_sound();

    is_scaled = 0;
    __asm__ __volatile__("sti");
}

void __attribute__((section(".text.entry"))) kernel_main() {
    clear_bss();
    init_paging();
    enable_paging();
    
    screen_clear();
    init_idt();
    boot();

	sti();
    current_mode = 0;

    create_task(task2_main);
    create_task(task3_main);
    create_task(hardware_check_main);

    while (1) {
        hlt();
    }
}
