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

int sys_exec(const char* filename) {
    return (int)_syscall(SYS_EXEC, (uint32_t)filename, 0, 0);
}

void sys_exit() {
    _syscall(SYS_EXIT, 0, 0, 0);
}


int sys_get_battery() {
    return (int)_syscall(SYS_GET_BATTERY, 0, 0, 0);
}

int sys_getpid() {
    return _syscall(SYS_GETPID, 0, 0, 0);
}

int sys_getuid() {
    return _syscall(SYS_GETUID, 0, 0, 0);
}

int sys_open(const char* path) {
    return (int)_syscall(SYS_OPEN, (uint32_t)path, 0, 0);
}

int sys_read(int fd, char* buf, uint32_t count) {
    return (int)_syscall(SYS_READ, (uint32_t)fd, (uint32_t)buf, (uint32_t)count);
}

int sys_recvfrom() {
    return (int)_syscall(SYS_RECVFROM, 0, 0, 0);
}

uint32_t sys_time() {
    return _syscall(SYS_TIME, 0, 0, 0);
}

void sys_uname(char *buffer) {
    _syscall(SYS_UNAME, (uint32_t)buffer, 0, 0);
}

int sys_write(int fd, const char* str, uint8_t color) {
    return (int)_syscall(SYS_WRITE, (uint32_t)fd, (uint32_t)str, (uint32_t)color);
}
