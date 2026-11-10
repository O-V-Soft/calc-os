#include <stdio.h>

__attribute__((section(".text.prologue")))
void _start() {
    int current_uid = _syscall(SYS_GETUID, 0, 0, 0);
    if (current_uid != 0) {
        print("reboot: Permission denied\n");
        _syscall(SYS_EXIT, 0, 0, 0);
        return; 
    } 
    
    _syscall(SYS_REBOOT, 0, 0, 0);
    _syscall(SYS_EXIT, 0, 0, 0);
}
