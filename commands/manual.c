#include <coms.h>
#include <riscv.h>
#include <video.h>
#include <stdint.h>
#include <keyboard.h>
#include <utils.h>

#if defined(__riscv)
Command commands[] = {
    {"cln", cln},
    {"help", help},
    {"exit", exit2},
    {"su", su},
    {"devices", devices2},
    {"lifetime", lifetime},
    {"whoami", whoami},
    {"status", status},
    {"reset", reset},
    {NULL, NULL}
};
#else
Command commands[] = {
    {"cln", cln},
    {"help", help},
    {"exit", exit2},
    {"su", su},
    {"casm", casm},
    {"empire", empire},
    {"cat", cat},
    {"touch", touch},
    {"send", send},
    {"forth", forth},
    {"cube", cube},
    {NULL, NULL}
};
#endif

int execute_command(char *line) {
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(line, commands[i].name) == 0) {
            commands[i].func();
            return 0;
        }
    }

    return -1;
}

void cln() {
    #if defined(__riscv)
        screen_clear();
        return;
    #else
        screen_clear();
        ncount = 1;
        return;
    #endif
}

void help() {
    #if defined(__riscv)
    print("Available commands:\n", COLOR_WHITE);
    print("  help - show this message\n", COLOR_WHITE);
    print("  status - print CPU status\n", COLOR_WHITE);
    print("  reset - reset the system\n", COLOR_WHITE);
    print("  devices - print PCI devices\n", COLOR_WHITE);
    print("  lifetime - system timer ticks\n", COLOR_WHITE);
    print("  su - change to root\n", COLOR_WHITE);
    print("  exit - change to user\n", COLOR_WHITE);
    print("  whoami - print current user\n", COLOR_WHITE);
    #else
    print("Available commands:\n", COLOR_WHITE);
    print("  help - show this message\n", COLOR_WHITE);
    print("  cln  - clear the screen\n", COLOR_WHITE);
    print("  ls  - list all files\n", COLOR_WHITE);
    print("  touch  - create a new file\n", COLOR_WHITE);
    print("  status - check system status\n", COLOR_WHITE);
    print("  cat - print file content\n", COLOR_WHITE);
    print("  devices - print PCI devices\n", COLOR_WHITE);
    print("  send - send a byte to the network\n", COLOR_WHITE);
    print("  behave - receive a byte from the network\n", COLOR_WHITE);
    print("  reboot - reboot the system\n", COLOR_WHITE);
    print("  forth - Forth interpreter\n", COLOR_WHITE);
    print("  empire - play imperial march\n", COLOR_WHITE);
    print("  umount - umount FS\n", COLOR_WHITE);
    print("  mount - mount FS\n", COLOR_WHITE);
    print("  casm - Calc Assembly Compiler\n", COLOR_WHITE);
    print("  su - change to root\n", COLOR_WHITE);
    print("  exit - change to user\n", COLOR_WHITE);
    print("  whoami - print current user\n", COLOR_WHITE);
    print("  cube - spin the cube\n", COLOR_WHITE);
    #endif
}

void exit2() {
    if (current_uid == 0) {
        current_uid = 1;
    } 
}

void su() {
    if (current_uid == 0) {
        return;
    } else {
        char pass_buf[32];
        print("Password: ", COLOR_WHITE);
        input_wait_string(pass_buf);
        print("\n", COLOR_WHITE);

        if (compare_strings(pass_buf, (char*)root_password)) {
            current_uid = 0;
        } else {
            print("su: Authentication failure\n", COLOR_WHITE);
        }
    }
}
