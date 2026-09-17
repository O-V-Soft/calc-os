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

int compare_strings(const char *str1, const char *str2) {
	int i = 0;
	while (str1[i] == str2[i]) {
		if (str1[i] == '\0') {
			return 1;
		}
		i++;
	}
	return 0;
}

void copy_string(char *dest, char *src) {
	int i = 0;
	while (src[i] != '\0') {
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
}

void name_clear() {
    for (int i = 0; i < 128; i++) {
        name[i] = 0;
    }
}

void content_clear() {
    for (int i = 0; i < 512; i++) {
        content[i] = 0;
    }
}

void format_fat_name(const char* src, char dest[11]) {
    for (int i = 0; i < 11; i++) {
        dest[i] = ' ';
    }

    int name_idx = 0;
    int ext_idx = 0;
    int i = 0;
    int in_ext = 0;

    while (src[i] != '\0' && (name_idx < 8 || ext_idx < 3)) {
        char c = src[i++];
        if (c == '.') {
            in_ext = 1;
            continue;
        }

        if (c >= 'a' && c <= 'z') {
            c -= 32;
        }

        if (!in_ext) {
            if (name_idx < 8) {
                dest[name_idx++] = c;
            }
        } else {
            if (ext_idx < 3) {
                dest[8 + ext_idx++] = c;
            }
        }
    }
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

int strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

char* strstr(const char* haystack, const char* needle) {
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && *h == *n) {
            h++;
            n++;
        }
        if (!*n) return (char*)haystack;
        haystack++;
    }
    return 0;
}

static int is_delimiter(char c, const char *delim) {
    while (*delim) {
        if (c == *delim) {
            return 1;
        }
        delim++;
    }
    return 0;
}

char *strtok(char *str, const char *delim) {
    static char *last_str = NULL;

    if (str != NULL) {
        last_str = str;
    }

    if (last_str == NULL || *last_str == '\0') {
        return NULL;
    }

    while (*last_str && is_delimiter(*last_str, delim)) {
        last_str++;
    }

    if (*last_str == '\0') {
        last_str = NULL;
        return NULL;
    }

    char *token_start = last_str;

    while (*last_str) {
        if (is_delimiter(*last_str, delim)) {
            *last_str = '\0'; 
            last_str++;     
            return token_start;
        }
        last_str++;
    }

    last_str = NULL;
    return token_start;
}

int strncmp(const char* a, const char* b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            return (a[i] < b[i]) ? -1 : 1;
        }
        if (a[i] == '\0') {
            return 0; 
        }
    }
    return 0; 
}
