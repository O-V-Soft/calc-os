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

void send_signal(int target_task_id, int signum) {
    if (signum <= 0 || signum >= NUM_SIGNALS) return;

    Task* target = current_task;
    if (!target) return;

    if (signum == SIGKILL) {
        delete_task(target_task_id); 
        return;
    }

    target->pending_signals = target->pending_signals | (1 << signum);
}

void check_signals(Task* task, uint32_t* registers_on_stack) {
    if (!task || task->pending_signals == 0) return;

    for (int signum = 1; signum < NUM_SIGNALS; signum++) {
        if (task->pending_signals & (1 << signum)) {
            task->pending_signals = task->pending_signals & ~(1 << signum);

            if (signum == SIGINT || signum == SIGILL || signum == SIGFPE || signum == SIGTERM) {
                delete_task(task->pid);
                return;
            }
        }
    }
}
