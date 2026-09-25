#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

void *mythread(void *arg) {
  	char* string = malloc(12);
  	strcpy(string,"hello world");
  
  	pthread_cleanup_push(free, string);
  	
  	while(1){
  		printf("%s\n", string);
  	}
  
  	pthread_cleanup_pop(1);
  	return NULL;
}

int main() {
  	pthread_t tid;
  	int err;
  
  	printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());
  
  	err = pthread_create(&tid, NULL, mythread, NULL);
  	if (err) {
  	    printf("main: pthread_create() failed: %s\n", strerror(err));
  		return -1;
  	}
  	
  	pthread_cancel(tid);
  
  	return 0;
}
