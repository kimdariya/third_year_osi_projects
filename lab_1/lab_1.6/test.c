#include <stdio.h>
#include <unistd.h>
#include "mythread.h"

void *work(void *a) {
    return a;
}

void *sleeper(void *a) {
    (void)a;
    sleep(2);
    return NULL;
}

void *waiter(void *a) {
    (void)a;
    mythread_t child;
    void *r;
    if (mythread_create(&child, sleeper, NULL)) {
        return NULL;
    }
    mythread_join(child, &r);
    return NULL;
}

int main(void) {
    mythread_t t;
    void *r;

    if (mythread_create(&t, work, (void *)42) || mythread_join(t, &r)) {
        return 1;
    }
    printf("join: %ld\n", (long)r);
    printf("join twice: %s\n", mythread_join(t, &r) == -1 ? "error (ok)" : "fail");

    if (mythread_create(&t, work, NULL) || mythread_detach(t)) {
        return 1;
    }
    printf("detach: ok\n");
    printf("join detached: %s\n", mythread_join(t, &r) == -1 ? "error (ok)" : "fail");

    if (mythread_create(&t, waiter, NULL)) {
        return 1;
    }
    
    if (mythread_cancel(t)) {
        return 1;
    }
    if (mythread_join(t, &r)) {
        return 1;
    }
    printf("cancel: %s\n", r == MYTHREAD_CANCELED ? "ok" : "fail");
    return 0;
}
