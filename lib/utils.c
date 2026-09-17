#include <fat.h>
#include <cmos.h>
#include <video.h>
#include <utils.h>
#include <mouse.h>
#include <keyboard.h>
#include <idt.h>
#include <stdint.h>
#include <ata.h>
#include <sound.h>
#include <pci.h>

void reboot() {
    while (inb(0x64) & 0x02);
    outb(0x64, 0xFE);

    for (;;);
}

int abs(int x) {
    return (x < 0) ? -x : x;
}

