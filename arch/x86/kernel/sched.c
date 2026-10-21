#include <stdint.h>
#include <idt.h>
#include <task.h>

uint32_t schedule(uint32_t current_esp) {
    if (!current_task) return current_esp;

    current_task->esp = current_esp;

    Task* next = current_task->next;
    int loops = 0;
    while (!next->is_active && loops < 10) {
        next = next->next;
        loops++;
    }

    current_task = next;
    timer_ticks++;

    return current_task->esp;
}
