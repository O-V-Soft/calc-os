#if !defined(__riscv)
#include <fat.h>
#include <ata.h>
#include <sound.h>
#include <cmos.h>
#endif

#include <video.h>
#include <utils.h>
#include <mouse.h>
#include <keyboard.h>
#include <idt.h>
#include <stdint.h>
#include <pci.h>

int atoi(char *str) {
	int res = 0;
	for (int i = 0; str[i] != '\0'; ++i) {
		if (str[i] >= '0' && str[i] <= '9') {
			res = res * 10 + str[i] - '0';
		}
	}
	return res;
}

void itoa(int n, char* s) {
    int i = 0;
    int is_negative = 0;

    if (n == 0) {
        s[i++] = '0';
        s[i] = '\0';
        return;
    }

    if (n < 0) {
        is_negative = 1;
        n = -n;
    }

    while (n > 0) {
        s[i++] = (n % 10) + '0';
        n /= 10;
    }

    if (is_negative) {
        s[i++] = '-';
    }

    s[i] = '\0';
    
    for (int j = 0; j < i / 2; j++) {
        char temp = s[j];
        s[j] = s[i - j - 1]; 
        s[i - j - 1] = temp;
    }
}

void htoa(int n, char str[]) {
    str[0] = '0';
    str[1] = 'x';
    const char *hex_chars = "0123456789ABCDEF";
    
    for (int i = 7; i >= 0; i--) {
        str[i + 2] = hex_chars[n & 0xF];
        n >>= 4;
    }
    str[10] = '\0';
}

int atoi_super(const char* str) {
    int res = 0;
    int i = 0;

    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r') {
        i++;
    }

    while (str[i] >= '0' && str[i] <= '9') {
        res = res * 10 + (str[i] - '0');
        i++;
    }

    return res;
}
