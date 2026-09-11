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

void sys_uname(char *buffer) {
    _syscall(SYS_UNAME, (uint32_t)buffer, 0, 0);
}