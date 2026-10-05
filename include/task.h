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
    uint8_t id;
    uint8_t state;
    uint8_t is_active;

    uint32_t pending_signals; 
    sig_handler_t signal_handlers[NUM_SIGNALS];
    
    struct task* next;             
} Task;

extern Task* current_task;

extern Task task_node_0;
extern Task task_node_1;
extern Task task_node_2;
extern Task task_node_3;

void check_signals(Task* task, uint32_t* registers_on_stack);
void send_signal(int target_task_id, int signum);

void task_init();
void create_task(int task_id);
void delete_task(int task_id);
uint32_t schedule(uint32_t current_esp);

void prepare_task2();
void prepare_task3();
void prepare_task4();

#endif 
