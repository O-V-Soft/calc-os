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

int sys_close(int fd) {
    return (int)_syscall(SYS_CLOSE, (uint32_t)fd, 0, 0);
}