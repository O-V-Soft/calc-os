#include <stdio.h>

__attribute__((section(".text.prologue")))
void _start() {
    int battery_ok = _syscall(SYS_GET_BATTERY, 0, 0, 0);
    if (battery_ok) {
        _syscall(SYS_WRITE, 1, (unsigned int)"Battery: OK\n", 15);
    } else {
        _syscall(SYS_WRITE, 1, (unsigned int)"Battery: BAD. Please insert a new CMOS battery\n", 15);
    }
    _syscall(SYS_EXIT, 0, 0, 0);
}
