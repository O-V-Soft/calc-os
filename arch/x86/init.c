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

extern uint32_t __bss_start[];
extern uint32_t __bss_safe_end[];

static void clear_bss(void) {
    for (uint32_t *address = __bss_start; address < __bss_safe_end; address++) {
        *address = 0;
    }
}

void boot() {
    is_scaled = 1;
    screen_clear();

    print("Scanning PCI...                              ", COLOR_WHITE);
    pci_scan();
    print("[OK]\n\n", COLOR_WHITE);

    print("Initializing memory manager...               ", COLOR_WHITE);
    init_memory_manager();
    print("[OK]\n\n", COLOR_WHITE);

    print("Initializing ethernet card...                ", COLOR_WHITE);
    int is_rtl8139_found = rtl8139_find();
    if (is_rtl8139_found >= 0) {
        rtl8139_init();
	    rtl8139_register();
        struct net_device* net_device = get_primary_net_device();
        print("[OK]\n\n", COLOR_WHITE);
    } else {
        print("[ERR]\n\n", COLOR_WHITE);
    }
    
    print("Mounting FAT12 filesystem...                 ", COLOR_WHITE);
    vfs_mount("/", &fat12_driver);
    print("[OK]\n\n", COLOR_WHITE);

    print("Initializing icons...                        ", COLOR_WHITE);
    icon_init();
    print("[OK]\n\n", COLOR_WHITE);

    delay_ticks(100);

    screen_clear();
    init_palette();
    draw_rect(0, 0, 1024, 768, COLOR_WHITE);

    x = 0;
    y = 10;

    is_scaled = 1;
    print("Welcome to CalcOS!", COLOR_BLACK);
    is_scaled = 2;
    play_startup_sound();

    is_scaled = 0;
    __asm__ __volatile__("sti");
}

void __attribute__((section(".text.entry"))) kernel_main() {
    // The bootloader loads the flat image but does not initialize its BSS.
    clear_bss();
    init_paging();
    enable_paging();
    
    __asm__ __volatile__("cli");
	screen_clear();
    init_idt();
    __asm__ __volatile__("sti");
	
    boot();
    current_mode = 0;

    create_task(task2_main);
    create_task(task3_main);
    create_task(task4_main);

    while (1) {
        __asm__ __volatile__("hlt");
    }
	
	while (1) {
		__asm__ __volatile__("hlt");
	}
}
