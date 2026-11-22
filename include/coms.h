#ifndef COMS_H
#define COMS_H
#include <stdint.h>

typedef struct {
    char* name;
    void (*func)();
} Command;

extern Command commands[];

int execute_command(char *line);

void cln();
void help();
void exit2();
void su();
void whoami();
void status();
void reset();

void casm();
void empire();
void cat();
void touch();
void forth();
void ping();
void devices2();
void lifetime();

void cube();

#endif
