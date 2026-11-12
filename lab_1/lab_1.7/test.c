#include <stdio.h>
#include "uthread.h"

static void *worker(void *arg) {
    for (int i = 0; i < 3; i++) {
        printf("%s: step %d\n", (char *)arg, i);
        uthread_yield();
    }
    return NULL;
}

static uthread_s t1, t2, t3;

int main(void) {
    if (uthread_create(&t1, worker, "thread 1") ||
        uthread_create(&t2, worker, "thread 2") ||
        uthread_create(&t3, worker, "thread 3")) {
        fprintf(stderr, "uthread_create failed\n");
        return 1;
    }
    while (!t1.finished || !t2.finished || !t3.finished){
        uthread_yield();
    }    
    printf("all threads are finished :)\n");
    return 0;
}
