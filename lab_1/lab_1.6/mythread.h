#ifndef MYTHREAD_H
#define MYTHREAD_H

typedef struct mythread *mythread_t;

#define MYTHREAD_CANCELED ((void *)-1)

int mythread_create(mythread_t *thread, void *(*start_routine)(void *), void *arg);
int mythread_join(mythread_t thread, void **retval);
int mythread_detach(mythread_t thread);
int mythread_cancel(mythread_t thread);

#endif
