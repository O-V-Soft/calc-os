#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#define NUM_SIGNALS 32

#define SIGINT   2 
#define SIGILL   4  
#define SIGFPE   8   
#define SIGKILL  9  
#define SIGTERM  15

typedef void (*sig_handler_t)(int);

typedef struct task {
    uint32_t esp;           
    uint32_t pid;    
    uint8_t state;
    uint8_t is_active;

    uint32_t pending_signals; 
    sig_handler_t signal_handlers[NUM_SIGNALS];
    
    struct task *next; 
} Task;            

extern Task *current_task;

void check_signals(Task *task, uint32_t *registers_on_stack);
void send_signal(int target_task_id, int signum);

void task_init();
void delete_task(int pid);
uint32_t schedule(uint32_t current_esp);
void create_task(void (*entry_point)());

void task2_main();
void task3_main();
void task4_main();

#endif
