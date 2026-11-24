#include <stddef.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "uthread.h"

#define MAX_THREADS 16

static uthread_s main_thread = { .used = 1 };
static uthread_t threads[MAX_THREADS] = { &main_thread };
static int count = 1, current = 0;

static int has_active_user_threads(void) {
    for (int i = 1; i < count; i++) {
        if (threads[i] != NULL && threads[i]->used && !threads[i]->finished) {
            return 1;
        }
    }
    return 0;
}

void uthread_yield(void) {
    int prev = current;
    int next = (current + 1) % count;

    while (next != prev && (threads[next] == NULL || !threads[next]->used || threads[next]->finished)) {
        next = (next + 1) % count;
    }

    if (next == prev || threads[next] == NULL || threads[next]->finished) {
        return;
    }

    current = next;
    swapcontext(&threads[prev]->ctx, &threads[next]->ctx);
}

static void alarm_handler(int sig) {
    (void)sig;
    uthread_yield();
}

static void wait_all_threads(void) {
    while (has_active_user_threads()) {
        pause();
    }
    ualarm(0, 0);
}

static void enable_preemption(void) {
    struct sigaction sa = {0};
    sa.sa_handler = alarm_handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);
    atexit(wait_all_threads);
    ualarm(1000, 1000);
}

static void trampoline(void) {
    uthread_t self = threads[current];
    self->fn(self->arg);
    self->finished = 1;
    self->used = 0;
    uthread_yield();
}

int uthread_create(uthread_t t, void *(*start_routine)(void *), void *arg) {
    static int timer_started = 0;
    if (!timer_started) {
        enable_preemption();
        timer_started = 1;
    }

    if (!t || !start_routine || getcontext(&t->ctx) == -1) {
        return -1;
    }

    int slot = -1;
    for (int i = 0; i < MAX_THREADS; i++) {
        if (threads[i] == NULL || !threads[i]->used) {
            slot = i;
            break;
        }
    }
    if (slot == -1) {
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
    if (slot >= count) {
        count = slot + 1;
    }

    return 0;
}
