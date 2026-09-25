#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

int global = 10;

void *mythread(void *arg) {
	int thread_num = (int)(long) arg;
	int local = 5;
	const int local_const = 6;
	static int local_static = 7;
	
	if(thread_num == 0){
		global = 20;
		local_static = 100;
		local = 1000;
	}	
	
	printf("\nmythread [PID = %d; PPID = %d; kernel_TID =  %d; posix_current_TID =  %lu]: Hello from mythread!\n",
	 getpid(), getppid(), gettid(),pthread_self());

	printf("\n global_value = %d;\n local_value = %d;\n local_const_value = %d;\n local_static_value = %d;\n", 
	global, local, local_const, local_static);	

	printf("\n gloval_address = %p;\n local_address = %p;\n local_const_address = %p;\n local_static_address = %p;\n ",
	(void*)&global, (void*)&local, (void*)&local_const, (void*)&local_static);

	return NULL;
}

int main() {
	pthread_t tid[5];
	int err;

	printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

	for (int i = 0; i < 5; i++){
		err = pthread_create(&tid[i], NULL, mythread, (void*)(long)i);
	    if (err) {
        	printf("main: pthread_create() failed: %s\n", strerror(err));
		    return -1;
        }
	}

	getchar(); 

	pthread_exit(NULL);

	return 0;
}
