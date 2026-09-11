#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <idt.h>
#include <task.h>
#include <stdint.h>
#include <sound.h>
#include <forth.h>

int sys_write(int fd, const char* str, uint8_t color) {
    return (int)_syscall(SYS_WRITE, (uint32_t)fd, (uint32_t)str, (uint32_t)color);
}