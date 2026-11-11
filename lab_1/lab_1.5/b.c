#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

void handler1(int sig) { write(1, "handler 1 (set by thread 1)\n", 28); }
void handler2(int sig) { write(1, "handler 2 (set by thread 2)\n", 28); }
void handler3(int sig) { write(1, "handler 3 (set by thread 3)\n", 28); }

static void set_handler(int sig, void (*h)(int)) {
    struct sigaction sa;
    sa.sa_handler = h;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(sig, &sa, NULL);
}

void *firstThread(void *arg) {
    set_handler(SIGUSR1, handler1); 
    printf("thread 1: set handler 1\n");
    sigset_t wait_set;
    sigfillset(&wait_set);
    sigdelset(&wait_set, SIGUSR1);
    sigsuspend(&wait_set); 
    return NULL;
}

void *secondThread(void *arg) {
    set_handler(SIGUSR1, handler2); 
    printf("thread 2: set handler 2\n");
    sigset_t wait_set;
    sigfillset(&wait_set);
    sigdelset(&wait_set, SIGUSR1);
    sigsuspend(&wait_set); 
    return NULL;
}

void *thirdThread(void *arg) {
    set_handler(SIGUSR1, handler3);
    printf("thread 3: set handler 3\n");
    sigset_t wait_set;
    sigfillset(&wait_set);
    sigdelset(&wait_set, SIGUSR1);
    sigsuspend(&wait_set); 
    return NULL;
}

int main() {
    pthread_t t1, t2, t3;
    int err;
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    err = pthread_create(&t1, NULL, firstThread, NULL);
    if (err) { printf("main: thread1 failed: %s\n", strerror(err)); return -1; }
    err = pthread_create(&t2, NULL, secondThread, NULL);
    if (err) { printf("main: thread2 failed: %s\n", strerror(err)); return -1; }
    err = pthread_create(&t3, NULL, thirdThread, NULL);
    if (err) { printf("main: thread3 failed: %s\n", strerror(err)); return -1; }

    sleep(1);

    pthread_kill(t1, SIGUSR1);
    pthread_kill(t2, SIGUSR1);
    pthread_kill(t3, SIGUSR1);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    return 0;
}
