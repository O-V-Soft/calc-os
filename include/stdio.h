#ifndef STDIO_H
#define STDIO_H

#define SYS_EXIT    1
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_TIME    13
#define SYS_GETPID  20
#define SYS_UNAME   122
#define SYS_CLOSE   6
#define SYS_EXEC    11
#define SYS_MOUNT   165
#define SYS_UMOUNT2 166
#define SYS_GETUID  24

#define SYS_GET_BATTERY  123
#define SYS_GET_TICKS    124
#define SYS_PCI_PRINT    125
#define SYS_LIST_FILES   126
#define SYS_CREATE_FILE  128
#define SYS_WAIT_STRING  129
#define SYS_REBOOT       130
#define SYS_BEHAVE       131

static inline int _syscall(int num, unsigned int arg1, unsigned int arg2, unsigned int arg3) {
    int ret;
    __asm__ __volatile__ ("int $0x80" : "=a" (ret) : "a" (num), "b" (arg1), "c" (arg2), "d" (arg3) : "memory");
    return ret;
}

#endif