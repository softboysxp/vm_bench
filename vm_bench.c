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
#include <sys/mman.h>
#include <string.h>
#include <signal.h>

#ifdef DEBUG
#include <assert.h>
#endif

#include "bench_op.h"

static int iter = 1000;
static int page_size = 0;
static void *temp_page = NULL;
static void *temp_buf = NULL;

static inline void vm_bench_mmap() {
	temp_page = mmap(NULL, page_size,
					 PROT_READ | PROT_WRITE,
					 MAP_PRIVATE | MAP_ANONYMOUS | MAP_LOCKED | MAP_POPULATE,
					 -1, 0);
#ifdef DEBUG
	assert(temp_page > 0);
#endif
}

static inline void vm_bench_fill() {
	memset(temp_page, 0, page_size);
}

static inline void vm_bench_copy() {
	memcpy(temp_page, temp_buf, page_size);
}

void seg_fault_handler(int sig) {
#ifdef DEBUG
	assert(sig == SIGSEGV);
#endif

	mprotect(temp_page, page_size, PROT_READ | PROT_WRITE);
}

static inline void vm_bench_seg_fault_handler() {
	mprotect(temp_page, page_size, PROT_NONE);
	*((char *) temp_page) = '\0';
}

static inline void vm_bench_munmap() {
#ifdef DEBUG
	assert(temp_page);
#endif

	munmap(temp_page, page_size);
}

void usage(const char *command) {
	fprintf(stderr, "Usage: %s [-i <iterations>]\n", command);
}

int main(int argc, char *argv[]) {
	int ch;

	while ((ch = getopt(argc, argv, "i:h?")) != -1) {
		switch (ch) {
			case 'i':
				iter = atoi(optarg);
				break;

			case '?':
			case 'h':
				usage(argv[0]);
				break;

			default:
				break;
		}
	}

	page_size = getpagesize();
#ifdef DEBUG
	assert(page_size);
	fprintf(stderr, "Page Size: %d\n", page_size);
#endif

	BENCH_OP(vm_bench_mmap);
	BENCH_OP(vm_bench_fill);

	temp_buf = malloc(page_size);
	BENCH_OP(vm_bench_copy);
	free(temp_buf);

	signal(SIGSEGV, seg_fault_handler);
	BENCH_OP(vm_bench_seg_fault_handler);
	signal(SIGSEGV, SIG_DFL);

	BENCH_OP(vm_bench_munmap);
}

