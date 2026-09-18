#include <stdint.h>
#include <riscv.h>
#include <mm.h>
#include <coms.h>
#include <keyboard.h>
#include <utils.h>
#include <video.h>

char com[512];
char buffer[4096];

void system();

void boot() {
    is_scaled = 1;
    screen_clear();

    print("Scanning PCI...                              ", COLOR_WHITE);
    pci_scan();
    print("[OK]\n\n", COLOR_WHITE);

	print("Initializing timer...                        ", COLOR_WHITE);
    init_timer();
	print("[OK]\n\n", COLOR_WHITE);
	
    print("Initializing memory manager...               ", COLOR_WHITE);
    init_memory_manager();
    print("[OK]\n\n", COLOR_WHITE);

    print("Initializing GPU...                          ", COLOR_WHITE);
    init_gpu();
    print("[OK]\n\n", COLOR_WHITE);

    screen_clear();
    draw_rect(0, 0, 1024, 768, COLOR_WHITE);

    x = 0;
    y = 10;

    is_scaled = 1;
    print("Welcome to CalcOS!", COLOR_BLACK);
    is_scaled = 0;
}

void system_riscv(uint32_t hartid, uint32_t dtb_ptr) {
    boot();
	current_mode = 0;
	system();
}
 
