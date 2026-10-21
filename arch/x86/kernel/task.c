#include <stdint.h>
#include <idt.h>
#include <task.h>
#include <mm.h>
#include <video.h>
#include <utils.h>
#include <forth.h>

Task *current_task = 0;
static int next_pid = 1;
static Task main_task;

void task_init() {
    main_task.pid = 0;
    main_task.esp = 0;
    main_task.is_active = 1;
    main_task.next = &main_task;

    current_task = &main_task;
}

void task2_main() {
    int hours, minutes;
    int old_hours = -1, old_minutes = -1;
    char h_str[3], m_str[3];

    while(1) {
        if (current_mode != 0) {
            get_time(&hours, &minutes);

            if (hours != old_hours || minutes != old_minutes) {
                draw_rect(965, 2, 55, 34, COLOR_LIGHT_GRAY); 
                draw_cube(993, 20, 110, 15, 15);

                cli();

                x = 980; y = 20;
                
                itoa(hours, h_str);
                if (hours < 10) print("0", COLOR_WHITE); 
                print(h_str, COLOR_WHITE);
                
                print(":", COLOR_WHITE);
                
                itoa(minutes, m_str);
                if (minutes < 10) print("0", COLOR_WHITE); 
                print(m_str, COLOR_WHITE);

                sti();

                old_hours = hours;
                old_minutes = minutes;
            }
        }
        __asm__ __volatile__("hlt");
    }
}

void task3_main() {
    while(1) {
        interpret(stack_init(1024), content);
    }
}

void task4_main() {
    system(); 
}

void create_task(void (*entry_point)()) {
    Task *new_task = (Task*)kmalloc(sizeof(Task));
    if (!new_task) return;

    uint8_t *stack = (uint8_t*)kmalloc(4096);
    if (!stack) {
        kfree(new_task);
        return;
    }
    
    uint32_t *sp = (uint32_t*)(stack + 4096);

    *(--sp) = 0x0202; 
    *(--sp) = 0x08;
    *(--sp) = (uint32_t)entry_point;

    *(--sp) = 0;
    *(--sp) = 0;

    for (int i = 0; i < 8; i++) {
        *(--sp) = 0;
    }

    *(--sp) = 0x10;

    new_task->esp = (uint32_t)sp;
    new_task->pid = next_pid++;
    new_task->is_active = 1;
    
    __asm__ __volatile__("cli");
    new_task->next = current_task->next;
    current_task->next = new_task;
    __asm__ __volatile__("sti");
}

void delete_task(int pid) {
    __asm__ __volatile__("cli");

    Task *curr = current_task;
    
    while (curr != current_task) {
        if (curr->pid == pid) {
            curr->is_active = 0;
            break;
        }
        curr = curr->next;
    };

    if (current_task->pid == pid) {
        __asm__ __volatile__("sti");
        while (1) {
            __asm__ __volatile__("hlt");
        }
    }

    __asm__ __volatile__("sti");
}
