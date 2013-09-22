/**
 * Copyright (c) 2013, University of Virginia
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#ifdef DEBUG
	#include <assert.h>
#endif

#include "bench_op.h"

static int iter = 1000;
static int num_threads = 16;

void *thread_create(void *arg) {
	return NULL;
}

static inline void pthreads_bench_create() {
	pthread_t tids[num_threads];

	for (int i = 0; i < num_threads; i++) {
		pthread_create(&tids[i], NULL, thread_create, NULL);
	}

	for (int i = 0; i < num_threads; i++) {
		void *ret;
		pthread_join(tids[i], &ret);
#ifdef DEBUG
		assert(ret == NULL);
#endif
	}
}

static pthread_mutex_t mutex_counter = PTHREAD_MUTEX_INITIALIZER;

void *thread_mutex(void *arg) {
	pthread_mutex_lock(&mutex_counter);
	(*((int *) arg))++;
	pthread_mutex_unlock(&mutex_counter);

	return arg;
}

static inline void pthreads_bench_mutex() {
	pthread_t tids[num_threads];
	int counter = 0;

	for (int i = 0; i < num_threads; i++) {
		pthread_create(&tids[i], NULL, thread_mutex, &counter);
	}

	for (int i = 0; i < num_threads; i++) {
		void *ret;
		pthread_join(tids[i], &ret);
	}

#ifdef DEBUG
	assert(counter == num_threads);
#endif
}

void *thread_sem(void *arg) {
	sem_t *sem = (sem_t *) arg;
	sem_wait(sem);

	return arg;
}

static inline void pthreads_bench_sem() {
	pthread_t tids[num_threads];

	sem_t sem;
	sem_init(&sem, 0, num_threads);

	for (int i = 0; i < num_threads; i++) {
		pthread_create(&tids[i], NULL, thread_sem, &sem);
	}

	for (int i = 0; i < num_threads; i++) {
		void *ret;
		pthread_join(tids[i], &ret);
	}

#ifdef DEBUG
	int val;
	sem_getvalue(&sem, &val);
	assert(val == 0);
#endif

	sem_destroy(&sem);
}

void usage(const char *command) {
	fprintf(stderr, "Usage: %s [-i <iterations>] [-n <no. of threads>]\n", command);
}

int main(int argc, char *argv[]) {
	int ch;

	while ((ch = getopt(argc, argv, "i:n:h?")) != -1) {
		switch (ch) {
			case 'i':
				iter = atoi(optarg);
				break;

			case 'n':
				num_threads = atoi(optarg);
				break;

			case '?':
			case 'h':
				usage(argv[0]);
				break;

			default:
				break;
		}
	}

	BENCH_OP(pthreads_bench_create);
	BENCH_OP(pthreads_bench_mutex);
	BENCH_OP(pthreads_bench_sem);

	pthread_mutex_destroy(&mutex_counter);
}

