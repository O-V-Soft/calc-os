#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <idt.h>
#include <task.h>
#include <stdint.h>
#include <sound.h>
#include <forth.h>

Task task_list[4];
int current_task = 0;

unsigned int task2_stack[1024]; 
unsigned int task3_stack[1024]; 
unsigned int task4_stack[2048]; 

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

void prepare_task2() {
    uint32_t* st = &task2_stack[1024];

    *(--st) = 0x202;       
    *(--st) = 0x08;    
    *(--st) = (uint32_t)task2_main; 

    *(--st) = 0;
    *(--st) = 0;

    for (int i = 0; i < 8; i++) {
        *(--st) = 0;
    }

    *(--st) = 0x10;

    task_list[1].esp = (void*)st;
}

void task3_main() {
    while(1) {
        interpret(stack_init(1024), content);
    }
}

void prepare_task3() {
    uint32_t* st = &task3_stack[1024];

    *(--st) = 0x202;    
    *(--st) = 0x08; 
    *(--st) = (uint32_t)task3_main; 

    *(--st) = 0;          
    *(--st) = 32;           

    for (int i = 0; i < 8; i++) {
        *(--st) = 0;
    }

    *(--st) = 0x10;

    task_list[2].esp = (void*)st;
}

void task4_main() {
    system(); 
}

void prepare_task4() {
    uint32_t* st = &task4_stack[2048];

    *(--st) = 0x202;                
    *(--st) = 0x08;             
    *(--st) = (uint32_t)task4_main; 

    *(--st) = 0;
    *(--st) = 0; 

    for (int i = 0; i < 8; i++) {
        *(--st) = 0; 
    }

    *(--st) = 0x10; 

    task_list[3].esp = (void*)st;
}

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
