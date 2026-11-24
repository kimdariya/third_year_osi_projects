#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>

#include "queue.h"

#define RED "\033[41m"
#define NOCOLOR "\033[0m"

typedef struct {
	queue_t *q;
	int cpu;
} worker_t;

void set_cpu(int n) {
	int err;
	cpu_set_t cpuset;
	pthread_t tid = pthread_self();

	CPU_ZERO(&cpuset);
	CPU_SET(n, &cpuset);

	err = pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset);
	if (err) {
		printf("set_cpu: pthread_setaffinity failed for cpu %d\n", n);
		return;
	}

	printf("set_cpu: set cpu %d\n", n);
}

void *reader(void *arg) {
	int expected = 0;
	worker_t *w = (worker_t *)arg;
	queue_t *q = w->q;

	printf("reader [%d %d %d]\n", getpid(), getppid(), gettid());

	set_cpu(w->cpu);

	while (1) {
		int val = -1;
		int ok = queue_get(q, &val);
		if (!ok)
			continue;

		if (expected != val)
			printf(RED "ERROR: get value is %d but expected - %d" NOCOLOR "\n", val, expected);

		expected = val + 1;
	}

	return NULL;
}

void *writer(void *arg) {
	int i = 0;
	worker_t *w = (worker_t *)arg;
	queue_t *q = w->q;

	printf("writer [%d %d %d]\n", getpid(), getppid(), gettid());

	set_cpu(w->cpu);

	while (1) {
		int ok = queue_add(q, i);
		if (!ok)
			continue;
		i++;
	}

	return NULL;
}

static int parse_int(const char *s, int *out) {
	char *end = NULL;
	long v;

	errno = 0;
	v = strtol(s, &end, 10);
	if (errno || !end || *end != '\0' || v < 0 || v > 100000000)
		return -1;

	*out = (int)v;
	return 0;
}

int main(int argc, char **argv) {
	pthread_t reader_tid;
	pthread_t writer_tid;
	queue_t *q;
	worker_t rw;
	worker_t ww;
	int max_count;
	int use_yield;
	int err;

	if (argc != 5) {
		printf("usage: %s <max_count> <reader_cpu> <writer_cpu> <yield 0|1>\n", argv[0]);
		return 1;
	}

	if (parse_int(argv[1], &max_count) || max_count <= 0) {
		printf("invalid max_count: %s\n", argv[1]);
		return 1;
	}
	if (parse_int(argv[2], &rw.cpu) || rw.cpu >= CPU_SETSIZE) {
		printf("invalid reader_cpu: %s\n", argv[2]);
		return 1;
	}
	if (parse_int(argv[3], &ww.cpu) || ww.cpu >= CPU_SETSIZE) {
		printf("invalid writer_cpu: %s\n", argv[3]);
		return 1;
	}
	if (parse_int(argv[4], &use_yield) || use_yield > 1) {
		printf("invalid yield flag: %s\n", argv[4]);
		return 1;
	}

	printf("main [%d %d %d]\n", getpid(), getppid(), gettid());

	q = queue_init(max_count);
	rw.q = q;
	ww.q = q;

	err = pthread_create(&reader_tid, NULL, reader, &rw);
	if (err) {
		printf("main: pthread_create() failed: %s\n", strerror(err));
		queue_destroy(q);
		return -1;
	}

	if (use_yield)
		sched_yield();

	err = pthread_create(&writer_tid, NULL, writer, &ww);
	if (err) {
		printf("main: pthread_create() failed: %s\n", strerror(err));
		pthread_cancel(reader_tid);
		pthread_join(reader_tid, NULL);
		queue_destroy(q);
		return -1;
	}

	err = pthread_join(reader_tid, NULL);
	if (err) {
		printf("main: pthread_join() failed: %s\n", strerror(err));
		return -1;
	}

	err = pthread_join(writer_tid, NULL);
	if (err) {
		printf("main: pthread_join() failed: %s\n", strerror(err));
		return -1;
	}

	queue_destroy(q);
	return 0;
}
