#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

struct thread_struct{
	int number;
	char *string;
};

void *mythread(void *arg) {
        struct thread_struct *data = (struct thread_struct*)arg;
	      printf("mythread [%d %d %d]: Hello from mythread!\n", getpid(), getppid(), gettid());
	      printf("%d , %s\n", data->number, data->string);
        return NULL;
}

int main() {
        pthread_t tid;
        int err;
	      struct thread_struct data = {1, "My thread! Wow!"};

        printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

        err = pthread_create(&tid, NULL, mythread, &data);
        if (err) {
            printf("main: pthread_create() failed: %s\n", strerror(err));
            return -1;
        }

	      pthread_join(tid, NULL);

        return 0;
}
