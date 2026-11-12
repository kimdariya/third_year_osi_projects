#define _GNU_SOURCE
#include <linux/futex.h>
#include <sched.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>
#include "mythread.h"

#define STACK_SIZE (1 << 20)
#define MAX_THREADS 64

struct mythread {
    volatile int tid;
    void *(*fn)(void *);
    void *arg;
    void *retval;
    void *stack;
    volatile int canceled;
    int used;
    int detached;
};

static struct mythread pool[MAX_THREADS];
static int handler_installed;
static void reap(void);

static volatile int reaper_started;

static void *reaper(void *arg){
    (void)arg;
    while (1) {
        reap();
        usleep(20000);
    }
    return NULL;
}

static void start_reaper(void){
    if (reaper_started){
        return;
    }
    reaper_started = 1;

    mythread_t t;
    if (mythread_create(&t, reaper, NULL) == 0){
        mythread_detach(t);
    }
}

static void on_cancel(int sig) {
    (void)sig;
}

static struct mythread *self(void) {
    int tid = syscall(SYS_gettid);
    for (int i = 0; i < MAX_THREADS; i++) {
        if (pool[i].used && pool[i].tid == tid) {
            return &pool[i];
        }
    }
    return NULL;
}

static void testcancel(void) {
    struct mythread *t = self();
    if (t && t->canceled) {
        t->retval = MYTHREAD_CANCELED;
        syscall(SYS_exit, 0);
    }
}

static int start(void *p) {
    struct mythread *t = p;
    t->retval = t->fn(t->arg);
    return 0;
}

static void reap(void) {
    for (int i = 0; i < MAX_THREADS; i++) {
        struct mythread *t = &pool[i];
        if (t->used && t->detached && t->tid == 0) {
            munmap(t->stack, STACK_SIZE);
            t->used = 0;
        }
    }
}

int mythread_create(mythread_t *thread, void *(*start_routine)(void *), void *arg) {
    if (!thread || !start_routine) {
        return -1;
    }
    reap();

    struct mythread *mt = NULL;
    for (int i = 0; i < MAX_THREADS; i++) {
        if (!pool[i].used) {
            mt = &pool[i];
            break;
        }
    }
    if (!mt) {
        return -1;
    }

    void *stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED) {
        return -1;
    }

    if (!handler_installed) {
        struct sigaction sa = { .sa_handler = on_cancel };
        sigemptyset(&sa.sa_mask);
        if (sigaction(SIGUSR1, &sa, NULL) == -1) {
            munmap(stack, STACK_SIZE);
            return -1;
        }
        handler_installed = 1;
        start_reaper();
    }

    mt->used = 1;
    mt->fn = start_routine;
    mt->arg = arg;
    mt->retval = NULL;
    mt->canceled = 0;
    mt->detached = 0;
    mt->stack = stack;

    int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND |
                CLONE_THREAD | CLONE_SYSVSEM |
                CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID;
    if (clone(start, (char *)stack + STACK_SIZE, flags, mt,
              &mt->tid, NULL, &mt->tid) == -1) {
        munmap(stack, STACK_SIZE);
        mt->used = 0;
        return -1;
    }

    *thread = mt;
    return 0;
}

int mythread_join(mythread_t thread, void **retval) {
    if (!thread || !thread->used || thread->detached) {
        return -1;
    }
    if (thread->tid == syscall(SYS_gettid)) {
        return -1;
    }

    int tid;
    while ((tid = thread->tid) != 0) {
        testcancel();
        struct timespec ts = { 0, 10 * 1000 * 1000 };
        syscall(SYS_futex, &thread->tid, FUTEX_WAIT, tid, &ts, NULL, 0);
    }

    if (retval) {
        *retval = thread->retval;
    }
    munmap(thread->stack, STACK_SIZE);
    thread->used = 0;
    return 0;
}

int mythread_detach(mythread_t thread) {
    if (!thread || !thread->used || thread->detached) {
        return -1;
    }
    thread->detached = 1;
    reap();
    return 0;
}

int mythread_cancel(mythread_t thread) {
    if (!thread || !thread->used) {
        return -1;
    }
    int tid = thread->tid;
    if (!tid) {
        return 0;
    }
    thread->canceled = 1;
    if (syscall(SYS_tgkill, getpid(), tid, SIGUSR1) == -1) {
        return -1;
    }
    return 0;
}
