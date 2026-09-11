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

int sys_read(int fd, char* buf, uint32_t count) {
    return (int)_syscall(SYS_READ, (uint32_t)fd, (uint32_t)buf, (uint32_t)count);
}