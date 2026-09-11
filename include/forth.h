#ifndef FORTH_H
#define FORTH_H
#include <stdint.h>

struct Stack;
struct Word;

struct Stack* stack_init(int size);
void stack_push(struct Stack* st, int value);
int stack_pop(struct Stack* st);

void word_add(struct Stack* st);
void word_sub(struct Stack* st);
void word_mul(struct Stack* st);
void word_div(struct Stack* st);
void word_dot(struct Stack* st);

struct Word* find_word(const char* token);
char* next_token(char** input);
int is_number(const char* s);
void interpret(struct Stack* st, char* input);

#endif