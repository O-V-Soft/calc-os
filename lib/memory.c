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

int memcmp(const char *s1, const char *s2, int n) {
    for (int i = 0; i < n; i++) {
        if (s1[i] != s2[i]) {
            return 0; 
        }
    }
    return 1; 
}

void* memcpy(void* dest, const void* src, uint32_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;

    for (uint32_t i = 0; i < n; i++) {
        d[i] = s[i];
    }

    return dest;
}

void* memset(void* s, int c, size_t n) {
    asm volatile (
        "cld\n\t"     
        "rep stosb\n\t" 
        :
        : "a"(c), "D"(s), "c"(n)
        : "memory"
    );
    return s;
}
