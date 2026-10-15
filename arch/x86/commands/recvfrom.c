#include <stdio.h>

void _start() {
    _syscall(SYS_RECVFROM, 0, 0, 0);
    _syscall(SYS_EXIT, 0, 0, 0);
}
