#include <stdio.h>

__attribute__((section(".text.prologue")))
void _start() {
    int current_uid = _syscall(SYS_GETUID, 0, 0, 0);
    if (current_uid != 0) {
        _syscall(SYS_WRITE, 1, (unsigned int)"user\n", 15);
        _syscall(SYS_EXIT, 0, 0, 0);
        return;
    } else {
        _syscall(SYS_WRITE, 1, (unsigned int)"root\n", 15);
        _syscall(SYS_EXIT, 0, 0, 0);
        return;
    }
}
