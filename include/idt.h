#ifndef IDT_H
#define IDT_H
#include <stdint.h>

struct idt_entry {
    uint16_t base_low;
    uint16_t selector;      
    uint8_t  always0;  
    uint8_t  flags;  
    uint16_t base_high; 
} __attribute__((packed));

struct idt_pointer {
    uint16_t idt_size;
    uint32_t idt_ptr;
} __attribute__((packed));

struct registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code; 
    uint32_t eip, cs, eflags, useresp, ss;
};

typedef struct {
    uint16_t cluster;
    uint32_t offset;
    uint8_t used;
} file_descriptor_t;

void init_idt();
void set_idt_gate(uint8_t number, uint32_t base, uint16_t selector, uint8_t flags);
void pic_remap();
void prepare_task4();
void exception_handler(struct registers *regs);

void create_task(int task_id);
void delete_task(int task_id);

extern void timer_wrapper();
extern void keyboard_wrapper();
extern void mouse_wrapper();
extern void ata_wrapper();

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

void delay_ticks(uint32_t ticks);
extern void syscall_wrapper();

extern unsigned int timer_ticks;
extern uint8_t *timer_str[16];

extern volatile int ata_interrupt_received;

extern volatile uint8_t mouse_packet[3];
extern volatile int mouse_ready;

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
#define SYS_GET_COLOR    132

#define EPERM            1
#define ENOENT           2
#define ESRCH            3
#define EINTR            4
#define EIO              5
#define ENXIO            6
#define E2BIG            7
#define ENOEXEC          8
#define EBADF            9
#define ECHILD          10
#define EAGAIN          11
#define ENOMEM          12
#define EACCES          13
#define EFAULT          14
#define ENOTBLK         15
#define EBUSY           16
#define EEXIST          17
#define EXDEV           18
#define ENODEV          19
#define ENOTDIR         20
#define EISDIR          21
#define EINVAL          22
#define ENFILE          23
#define EMFILE          24
#define ENOTTY          25
#define ETXTBSY         26
#define EFBIG           27
#define ENOSPC          28
#define ESPIPE          29
#define EROFS           30
#define EMLINK          31
#define EPIPE           32
#define EDOM            33
#define ERANGE          34

#define ENOTSOCK        88
#define EDESTADDRREQ    89
#define EMSGSIZE        90
#define EPROTOTYPE      91
#define ENOPROTOOPT     92
#define EPROTONOSUPPORT 93
#define ESOCKTNOSUPPORT 94
#define EOPNOTSUPP      95
#define EPFNOSUPPORT    96
#define EAFNOSUPPORT    97
#define EADDRINUSE      98
#define EADDRNOTAVAIL   99
#define ENETDOWN        100
#define ENETUNREACH     101
#define ECONNRESET      104
#define EISCONN         106
#define ENOTCONN        107
#define ETIMEDOUT       110
#define ECONNREFUSED    111

#define ENOSYS          38

uint32_t syscall_handler(struct registers *regs);

extern unsigned int task2_stack[1024];
extern unsigned int task3_stack[1024];
extern unsigned int task4_stack[2048];

static inline int _syscall(int num, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    int ret;
    __asm__ __volatile__ ("int $0x80" : "=a" (ret) : "a" (num), "b" (arg1), "c" (arg2), "d" (arg3) : "memory");
    return ret;
}

int sys_close(int fd);
int sys_exec(const char* filename);
void sys_exit();
int sys_get_battery();
int sys_getpid();
int sys_getuid();
int sys_open(const char* path);
int sys_read(int fd, char* buf, uint32_t count);
uint32_t sys_time();
void sys_uname(char *buffer);
int sys_write(int fd, const char* str, uint8_t color);

void system();


#define cli() __asm__ __volatile__ ("cli");
#define sti() __asm__ __volatile__ ("sti");

#endif
