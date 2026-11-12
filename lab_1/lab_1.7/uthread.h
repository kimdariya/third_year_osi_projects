#ifndef UTHREAD_H
#define UTHREAD_H

#include <ucontext.h>

#define UTHREAD_STACK_SIZE 32768

typedef struct uthread {
    ucontext_t ctx;
    char stack[UTHREAD_STACK_SIZE];
    void *(*fn)(void *);
    void *arg;
    int finished;
    int used;
} uthread_s, *uthread_t;

int uthread_create(uthread_t thread, void *(*start_routine)(void *), void *arg);

void uthread_yield(void);

#endif
