#include <stdio.h>

__attribute__((section(".text.prologue")))
void _start() {
    int current_uid = _syscall(SYS_GETUID, 0, 0, 0);
    if (current_uid != 0) {
        _syscall(SYS_WRITE, 1, (unsigned int)"umount: Permission denied\n", 15);
        _syscall(SYS_EXIT, 0, 0, 0);
        return; 
    } 
    
    char path[256];

    _syscall(SYS_WRITE, 1, (unsigned int)"Enter path: ", 15);
    _syscall(SYS_WAIT_STRING, (unsigned int)path, 0, 0);
    _syscall(SYS_WRITE, 1, (unsigned int)"\n", 15);

    int res = _syscall(SYS_UMOUNT2, (unsigned int)path, 0, 0);
    _syscall(SYS_EXIT, 0, 0, 0);
}
