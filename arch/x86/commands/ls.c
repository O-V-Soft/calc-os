#include <stdio.h>

__attribute__((section(".text.prologue")))
void _start() {
    _syscall(SYS_LIST_FILES, 0, 0, 0);
    _syscall(SYS_EXIT, 0, 0, 0);
}
