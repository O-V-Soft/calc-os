#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>

int compare_strings(const char *str1, const char *str2);

void copy_string(char *dest, char *src);

int atoi(char *str);
void itoa(int n, char* s);
void htoa(int n, char str[]);
#if !defined(__riscv)
void draw_button(int _x, int _y, int _width, int _height, const char *_msg, uint8_t color, uint8_t text_color);
#else
void draw_button(int _x, int _y, int _width, int _height, const char *_msg, uint32_t color, uint32_t text_color);
#endif

int abs(int x);
int min4(int a, int b, int c, int d);
int max4(int a, int b, int c, int d);

uint8_t check_battery();
uint8_t bcd_to_bin(uint8_t val);
void get_time(int *h, int *m);
int memcmp(const char *s1, const char *s2, int n);
void* memcpy(void* dest, const void* src, uint32_t n);
void reboot();
void name_clear();
void content_clear();
void format_fat_name(const char* src, char dest[11]);
void draw_file_icons();

void play_startup_sound();
void play_imperial_march();

int atoi_super(const char* str);

size_t strlen(const char* str);
int strcmp(const char* a, const char* b);
char* strstr(const char* haystack, const char* needle);
void* memset(void* s, int c, size_t n);

static int is_delimiter(char c, const char *delim);
char *strtok(char *str, const char *delim);

int strncmp(const char* a, const char* b, int n);

#endif
