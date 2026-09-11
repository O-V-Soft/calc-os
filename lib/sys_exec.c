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

int sys_exec(const char* filename) {
    return (int)_syscall(SYS_EXEC, (uint32_t)filename, 0, 0);
}