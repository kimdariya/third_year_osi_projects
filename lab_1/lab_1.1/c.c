#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

int global = 10;

void *mythread(void *arg) {
	pthread_t created_tid = *(pthread_t*)arg;
	int local = 5;
	const int local_const = 6;
	static int local_static = 7;
	
	printf("\nmythread [PID = %d; PPID = %d; TID =  %d; current_TID =  %lu]: Hello from mythread!\n",
	 getpid(), getppid(), gettid(), pthread_self());

	printf(" global_address = %p;\n local_address = %p;\n local_const_address = %p;\n local_static_address = %p;\n ",
	(void*)&global, (void*)&local, (void*)&local_const, (void*)&local_static);
	
	printf(" pthread_create returned: %lu\n", (unsigned long)created_tid);
	printf(" pthread_equal(self, created) = %d\n", pthread_equal(pthread_self(), created_tid));

	return NULL;
}

int main() {
	pthread_t tid[5];
	int err;

	printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

	for (int i = 0; i<5; i++){
		err = pthread_create(&tid[i], NULL, mythread, &tid[i]);
	    if (err) {
        	 printf("main: pthread_create() failed: %s\n", strerror(err));
             return -1;
        }
	}

	pthread_exit(NULL);

	return 0;
}
