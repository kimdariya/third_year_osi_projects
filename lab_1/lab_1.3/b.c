#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

struct thread_struct{
	int number;
	char *string;
};

void *mythread(void *arg) {
        struct thread_struct *data = (struct thread_struct*)arg;
      	printf("mythread [%d %d %d]: Hello from mythread!\n", getpid(), getppid(), gettid());
	      printf("%d , %s\n", data->number, data->string);
	      free(data);
	      return NULL;
}

int main() {
        pthread_t tid;
        int err;
	      pthread_attr_t attr;
	      struct thread_struct *data = malloc(sizeof(struct thread_struct));
    	  if (data == NULL) {
        	printf("Error! Memory allocation error\n");
        	return -1;
    	  }
    
    	  data->number = 1;
    	  data->string = "My thread! Wow!";;

        printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

	      pthread_attr_init(&attr);
	      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        err = pthread_create(&tid, &attr, mythread, data);
        if (err) {
            printf("main: pthread_create() failed: %s\n", strerror(err));
            pthread_attr_destroy(&attr);
	          free(data);
	          return -1;
        }

	      pthread_attr_destroy(&attr);
	      sleep(1);
  
        return 0;
}
