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

int sys_open(const char* path) {
    return (int)_syscall(SYS_OPEN, (uint32_t)path, 0, 0);
}