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

int min4(int a, int b, int c, int d) {
    int min = a;
    if (b < min) min = b;
    if (c < min) min = c;
    if (d < min) min = d;
    return min;
}

int max4(int a, int b, int c, int d) {
    int max = a;
    if (b > max) max = b;
    if (c > max) max = c;
    if (d > max) max = d;
    return max;
}

int interpolate_x(int y, int x1, int y1, int x2, int y2) {
    if (y1 == y2) return x1;
    return x1 + ((y - y1) * (x2 - x1)) / (y2 - y1);
}
