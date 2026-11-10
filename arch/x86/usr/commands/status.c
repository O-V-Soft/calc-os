#include <stdio.h>

__attribute__((section(".text.prologue")))
void _start() {
    int battery_ok = _syscall(SYS_GET_BATTERY, 0, 0, 0);
    if (battery_ok) {
        print("Battery: OK\n");
    } else {
        print("Battery: BAD. Please insert a new CMOS battery\n");
    }
    _syscall(SYS_EXIT, 0, 0, 0);
}
