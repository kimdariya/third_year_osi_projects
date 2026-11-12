#include <stddef.h>
#include "uthread.h"

#define MAX_THREADS 16

static uthread_s main_thread = { .used = 1 };
static uthread_t threads[MAX_THREADS] = { &main_thread };
static int count = 1, current = 0;

void uthread_yield(void) {
    int prev = current;
    int next = (current + 1) % count;

    while (threads[next]->finished && next != prev) {
        next = (next + 1) % count;
    }

    if (next == prev){
        return;
    }
    current = next;
    swapcontext(&threads[prev]->ctx, &threads[next]->ctx);
}

static void trampoline(void) {
    uthread_t self = threads[current];
    self->fn(self->arg);
    self->finished = 1;
    self->used = 0;
    uthread_yield();
}

int uthread_create(uthread_t t, void *(*start_routine)(void *), void *arg) {
    if (!t || !start_routine || getcontext(&t->ctx) == -1){
        return -1;
    }

    int slot = -1;
    for (int i = 0; i < MAX_THREADS; i++) {
        if (threads[i] == NULL || !threads[i]->used) {
            slot = i;
            break;
        }
    }
    if (slot == -1){
        return -1;
    }

    t->fn = start_routine;
    t->arg = arg;
    t->finished = 0;
    t->used = 1;

    t->ctx.uc_stack.ss_sp = t->stack;
    t->ctx.uc_stack.ss_size = UTHREAD_STACK_SIZE;
    t->ctx.uc_link = NULL;
    makecontext(&t->ctx, trampoline, 0);
    
    threads[slot] = t;
    if (slot >= count){
        count = slot + 1;
    }

    return 0;
}
