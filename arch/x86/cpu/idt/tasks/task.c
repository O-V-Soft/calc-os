#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <idt.h>
#include <task.h>
#include <stdint.h>
#include <sound.h>

Task task_list[4];
int current_task = 0;

void create_task(int task_id) {
    if (task_id < 1 || task_id > 3) return; 

    __asm__ __volatile__("cli"); 

    if (task_id == 1) {
        prepare_task2();
    } else if (task_id == 2) {
        prepare_task3();
    } else if (task_id == 3) {
        prepare_task4();
    }

    task_list[task_id].id = task_id;
    task_list[task_id].is_active = 1; 

    __asm__ __volatile__("sti");
}

void delete_task(int task_id) {
    if (task_id < 1 || task_id > 2) return;

    __asm__ __volatile__("cli");

    task_list[task_id].is_active = 0;

    if (current_task == task_id) {
        __asm__ __volatile__("sti");
        while(1) {
            __asm__ __volatile__("hlt"); 
        }
    }

    __asm__ __volatile__("sti");
}
