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

struct idt_entry   idt_entry[256];
struct idt_pointer idtp;

unsigned int timer_ticks = 0;
unsigned char *timer_str[16];

volatile int ata_interrupt_received = 0;

void set_idt_gate(uint8_t number, uint32_t base, uint16_t selector, uint8_t flags) {
    idt_entry[number].base_low = (base & 0xFFFF);
    idt_entry[number].base_high = (base >> 16) & 0xFFFF;
    idt_entry[number].selector = selector;
    idt_entry[number].always0 = 0;
    idt_entry[number].flags = flags;
}

void pic_remap() {
    outb(0x20, 0x11); 
    outb(0xA0, 0x11);
    
    outb(0x21, 0x20);
    outb(0xA1, 0x28); 
    
    outb(0x21, 0x04); 
    outb(0xA1, 0x02); 
    
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    
    outb(0x21, 0xF8); 
    outb(0xA1, 0x2F); 
}

uint32_t timer_handler(struct registers *regs) {
    check_signals(current_task, (uint32_t*)regs);

    if (task_list[current_task].is_active == 0) {
        int next_task = current_task;
        while (1) {
            next_task = (next_task + 1) % MAX_TASKS;
            if (next_task == 0 || task_list[next_task].is_active == 1) {
                break;
            }
        }
        current_task = next_task;
        timer_ticks++;
        outb(0x20, 0x20);
        return (uint32_t)task_list[current_task].esp; 
    }

    task_list[current_task].esp = (void*)regs; 

    int next_task = current_task;
    while (1) {
        next_task = (next_task + 1) % MAX_TASKS;
        if (next_task == 0 || task_list[next_task].is_active == 1) {
            break;
        }
    }

    current_task = next_task;
    timer_ticks++;
    outb(0x20, 0x20); 

    return (uint32_t)task_list[current_task].esp;
}

void delay_ticks(uint32_t ticks) {
    uint32_t target_ticks = timer_ticks + ticks;
    while (timer_ticks < target_ticks) {
        __asm__ __volatile__("hlt");
    }
}

void exception_handler(struct registers *regs) {
    if (current_task != 0) {
        printk("\nprocess with PID ", COLOR_WHITE);
        char num_buf[16];
        itoa(current_task, num_buf);
        printk(num_buf, COLOR_WHITE);
        printk(" crashed.\n", COLOR_WHITE);
        
        task_list[current_task].is_active = 0;

        if (current_task == 3) {
            is_crushed = 1;
            prepare_task4();       
            task_list[3].is_active = 1; 
        }

        outb(0x20, 0x20);
        outb(0xA0, 0x20);

        int next_task = current_task;
        while (1) {
            next_task = (next_task + 1) % MAX_TASKS;
            if (next_task == 0 || task_list[next_task].is_active == 1) {
                break;
            }
        }

        current_task = next_task;
        timer_ticks++;

        uint32_t new_stack = (uint32_t)task_list[current_task].esp;

        __asm__ __volatile__("mov %0, %%eax" : : "r"(new_stack));
        return; 
    }
    
    draw_rect(0, 0, 1024, 768, COLOR_BLUE); 

    x = 10;
    y = 10;
    
    printk("KERNEL FATAL: ", COLOR_WHITE);
    
    if (regs->int_no == 0) printk("DIVISION BY ZERO", COLOR_WHITE);
    else if (regs->int_no == 1) printk("DEBUG", COLOR_WHITE);
    else if (regs->int_no == 2) printk("NON MASKABLE INTERRUPT", COLOR_WHITE);
    else if (regs->int_no == 3) printk("BREAKPOINT", COLOR_WHITE);
    else if (regs->int_no == 4) printk("INTO DETECTED OVERFLOW", COLOR_WHITE);
    else if (regs->int_no == 5) printk("OUT OF BOUNDS", COLOR_WHITE);
    else if (regs->int_no == 6) printk("INVALID OPCODE", COLOR_WHITE);
    else if (regs->int_no == 7) printk("NO COPROCESSOR", COLOR_WHITE);
    else if (regs->int_no == 8) printk("DOUBLE FAULT", COLOR_WHITE);
    else if (regs->int_no == 9) printk("COPROCESSOR SEGMENT OVERRUN", COLOR_WHITE);
    else if (regs->int_no == 10) printk("BAD TSS", COLOR_WHITE);
    else if (regs->int_no == 11) printk("SEGMENT NOT PRESENT", COLOR_WHITE);
    else if (regs->int_no == 12) printk("STACK FAULT", COLOR_WHITE);
    else if (regs->int_no == 13) printk("GENERAL PROTECTION FAULT", COLOR_WHITE);
    else if (regs->int_no == 14) printk("PAGE FAULT", COLOR_WHITE);
    else if (regs->int_no == 15) printk("UNKNOWN INTERRUPT (RESERVED)", COLOR_WHITE);
    else if (regs->int_no == 16) printk("FLOATING POINT ERROR", COLOR_WHITE);
    else if (regs->int_no == 17) printk("ALIGNMENT CHECK", COLOR_WHITE);
    else if (regs->int_no == 18) printk("MACHINE CHECK", COLOR_WHITE);
    else if (regs->int_no == 19) printk("SIMD FLOATING POINT EXCEPTION", COLOR_WHITE);
    else if (regs->int_no == 20) printk("VIRTUALIZATION EXCEPTION", COLOR_WHITE);
    else if (regs->int_no == 21) printk("CONTROL PROTECTION EXCEPTION", COLOR_WHITE);
    else if (regs->int_no >= 22 && regs->int_no <= 27) printk("RESERVED EXCEPTION", COLOR_WHITE);
    else if (regs->int_no == 28) printk("HYPERVISOR INJECTION EXCEPTION", COLOR_WHITE);
    else if (regs->int_no == 29) printk("VMM COMMUNICATION EXCEPTION", COLOR_WHITE);
    else if (regs->int_no == 30) printk("SECURITY EXCEPTION", COLOR_WHITE);
    else if (regs->int_no == 31) printk("RESERVED EXCEPTION", COLOR_WHITE);
    else if (regs->int_no == 29) printk("VMM COMMUNICATION EXCEPTION", COLOR_WHITE);
    else if (regs->int_no == 30) printk("SECURITY EXCEPTION", COLOR_WHITE);
    else if (regs->int_no == 31) printk("RESERVED EXCEPTION", COLOR_WHITE);
    else printk("UNKNOWN EXCEPTION\n", COLOR_WHITE);

    x = 10;
    y = 70;
    printk("TECHICAL INFORMATION: ", COLOR_WHITE);
    char buf[16];
    itoa(regs->int_no, buf);
    printk("\n  INTERRUPT NO: ", COLOR_WHITE);
    printk(buf, COLOR_WHITE); 


    printk("\n  EIP: ", COLOR_WHITE);
    char eip_buf[32];
    htoa(regs->eip, eip_buf);
    printk(eip_buf, COLOR_WHITE);

    printk("\n  CS: ", COLOR_WHITE);
    htoa(regs->cs, buf); 
    printk(buf, COLOR_WHITE);

    printk("\n  ERR CODE: ", COLOR_WHITE);
    itoa(regs->err_code, buf); 
    printk(buf, COLOR_WHITE);
    
    while(1);
}

void init_timer() {
    outb(0x43, 0x36); 

    outb(0x40, 0x52); 
    outb(0x40, 0x09);
}

unsigned int task2_stack[1024]; 

void task2_main() {
    int hours, minutes;
    int old_hours = -1, old_minutes = -1;
    char h_str[3], m_str[3];

    while(1) {
        if (current_mode != 0 && is_window_crt == 0) {
            get_time(&hours, &minutes);

            if (hours != old_hours || minutes != old_minutes) {
                draw_rect(970, 5, 46, 30, COLOR_BLACK); 

                x = 972; y = 15;
                
                itoa(hours, h_str);
                if (hours < 10) print("0", COLOR_WHITE); 
                print(h_str, COLOR_WHITE);
                
                print(":", COLOR_WHITE);
                
                itoa(minutes, m_str);
                if (minutes < 10) print("0", COLOR_WHITE); 
                print(m_str, COLOR_WHITE);

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

unsigned int task3_stack[1024]; 

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

unsigned int task4_stack[2048]; 

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

void init_idt() {
    idtp.idt_ptr = (uint32_t)&idt_entry;
    idtp.idt_size = (sizeof(struct idt_entry) * 256) - 1;

    for (int i = 0; i < 256; i++) {
        set_idt_gate(i, 0, 0, 0);
    }

    pic_remap();
    init_timer();

    set_idt_gate(0,  (uint32_t)isr0,  0x08, 0x8E);
    set_idt_gate(1,  (uint32_t)isr1,  0x08, 0x8E);
    set_idt_gate(2,  (uint32_t)isr2,  0x08, 0x8E);
    set_idt_gate(3,  (uint32_t)isr3,  0x08, 0x8E);
    set_idt_gate(4,  (uint32_t)isr4,  0x08, 0x8E);
    set_idt_gate(5,  (uint32_t)isr5,  0x08, 0x8E);
    set_idt_gate(6,  (uint32_t)isr6,  0x08, 0x8E);
    set_idt_gate(7,  (uint32_t)isr7,  0x08, 0x8E);
    set_idt_gate(8,  (uint32_t)isr8,  0x08, 0x8E);
    set_idt_gate(9,  (uint32_t)isr9,  0x08, 0x8E);
    set_idt_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    set_idt_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    set_idt_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    set_idt_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    set_idt_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    set_idt_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    set_idt_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    set_idt_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    set_idt_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    set_idt_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    set_idt_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    set_idt_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    set_idt_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    set_idt_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    set_idt_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    set_idt_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    set_idt_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    set_idt_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    set_idt_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    set_idt_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    set_idt_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    set_idt_gate(31, (uint32_t)isr31, 0x08, 0x8E);

    set_idt_gate(32, (uint32_t)timer_wrapper, 0x08, 0x8E);
    set_idt_gate(33, (uint32_t)keyboard_wrapper, 0x08, 0x8E);
    set_idt_gate(44, (uint32_t)mouse_wrapper, 0x08, 0x8E);
    set_idt_gate(46, (uint32_t)ata_wrapper, 0x08, 0x8E);

    set_idt_gate(128, (uint32_t)syscall_wrapper, 0x08, 0x8E); 

    prepare_task2();
    prepare_task3();
    prepare_task4();
    
    current_task = 0; 
    task_list[0].id = 0;

    task_list[0].is_active = 1; 

    task_list[1].is_active = 1;
    task_list[2].is_active = 0;
    task_list[3].is_active = 0;

    __asm__ __volatile__("lidt (%0)" : : "r" (&idtp));

    for (int i = 0; i < 50; i++) {
        if (inb(0x64) & 0x01) {
            inb(0x60);
        }
    }

    init_mouse();

    mouse_cycle = 0; 
    mouse_ready = 0;
    
    __asm__ __volatile__("sti");
}
