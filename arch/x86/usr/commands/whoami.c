#include <stdio.h>

__attribute__((section(".text.prologue")))
void _start() {
    int current_uid = _syscall(SYS_GETUID, 0, 0, 0);
    if (current_uid != 0) {
        print("user\n");
        _syscall(SYS_EXIT, 0, 0, 0);
        return;
    } else {
        print("root\n");
        _syscall(SYS_EXIT, 0, 0, 0);
        return;
    }
}
