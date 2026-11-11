#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

void sigint_handler(int sig) {
    write(1, "thread 2: received SIGINT\n", 26);
}

void *firstThread(void *arg) {
    sigset_t set;
    sigfillset(&set);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    printf("thread 1: signals blocked\n");
    return NULL;
}

void *secondThread(void *arg) {
    sigset_t wait_set;
    struct sigaction sa;

    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    sigfillset(&wait_set);
    sigdelset(&wait_set, SIGINT);

    printf("thread 2: waiting for SIGINT\n");
    sigsuspend(&wait_set);
    return NULL;
}

void *thirdThread(void *arg) {
    sigset_t set;
    int sig;
    sigemptyset(&set);
    sigaddset(&set, SIGQUIT);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    printf("thread 3: waiting for SIGQUIT\n");
    sigwait(&set, &sig);
    printf("thread 3: received SIGQUIT\n");
    return NULL;
}

int main() {
    pthread_t t1, t2, t3;
    int err;
    sigset_t set;
    sigfillset(&set);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    err = pthread_create(&t1, NULL, firstThread, NULL);
    if (err) { printf("main: thread1 failed: %s\n", strerror(err)); return -1; }
    err = pthread_create(&t2, NULL, secondThread, NULL);
    if (err) { printf("main: thread2 failed: %s\n", strerror(err)); return -1; }
    err = pthread_create(&t3, NULL, thirdThread, NULL);
    if (err) { printf("main: thread3 failed: %s\n", strerror(err)); return -1; }

    pthread_kill(t1, SIGUSR1);
    pthread_kill(t2, SIGINT);
    pthread_kill(t3, SIGQUIT);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    return 0;
}
