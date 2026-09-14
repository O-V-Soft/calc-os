#include <coms.h>
#include <riscv.h>
#include <video.h>
#include <stdint.h>
#include <keyboard.h>
#include <utils.h>

void devices2() {
  pci_print_devices();
}

void lifetime() {
  itoa(timer_ticks, buffer);
  printk(buffer, COLOR_WHITE);
  printk("\n", COLOR_WHITE);
}

void whoami() {
    if (current_uid != 0) {
        printk("user\n", COLOR_WHITE);
    } else {
        printk("root\n", COLOR_WHITE);
    }
}

void status() {
    uint32_t mstatus = read_mstatus();
    uint32_t mie = read_mie();
    uint32_t hartid = read_mhartid(); 

    char buf[32];
        
    printk("Current Hart (Core) ID: ", COLOR_WHITE);
    itoa(hartid, buf); printk(buf, COLOR_WHITE); printk("\n", COLOR_WHITE);

    printk("mstatus register: ", COLOR_WHITE);
    htoa(mstatus, buf); printk(buf, COLOR_WHITE); printk("\n", COLOR_WHITE);

    printk("Global Interrupts: ", COLOR_WHITE);
    if (mstatus & (1 << 3)) {
        printk("ENABLED\n", COLOR_WHITE);
    } else {
        printk("DISABLED\n", COLOR_WHITE);
    }

    uint8_t mpp = (mstatus >> 11) & 0x3;
    printk("Previous Privilege Mode: ", COLOR_WHITE);
    if (mpp == 3) printk("Machine Mode (M-Mode)\n", COLOR_WHITE);
    else if (mpp == 1) printk("Supervisor Mode (S-Mode)\n", COLOR_WHITE);
    else printk("User Mode (U-Mode)\n", COLOR_WHITE);

    printk("Enabled Interrupts (mie): ", COLOR_WHITE);
    htoa(mie, buf); printk(buf, COLOR_WHITE); printk("\n\n", COLOR_WHITE);
}

void reset() {
    if (current_uid != 0) {
        printk("reset: Permission denied\n", COLOR_WHITE);
        return;
    }

    volatile uint32_t *sifive_finisher = (volatile uint32_t *)0x100000;
    *sifive_finisher = 0x5555;

    volatile uint32_t *syscon_reset = (volatile uint32_t *)0x1000000;
    *syscon_reset = 0x7777; 

    volatile uint32_t *watchdog_ctrl = (volatile uint32_t *)0x10007000;
    if (watchdog_ctrl) {
        *watchdog_ctrl = 0x1; 
    }

    asm volatile("csrw mtvec, zero");
        
    asm volatile("ebreak");
                
    while(1) {
        asm volatile("wfi"); 
    }
}
