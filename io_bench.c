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
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "bench_op.h"

static const int iter = 1;
static int block_size = (512 << 10);
static void *temp_buffer = NULL;
static size_t total_size = (1UL << 30);
static const char *temp_filename = NULL;

void clear_temp_buffer_cache() {
	int fd = open("/proc/sys/vm/drop_caches", O_WRONLY);
	write(fd, "3", 1);
	close(fd);
}

static inline void io_bench_write() {
	int fd = creat(temp_filename, 0600);
	size_t bytes_written = 0;
	
	while ((bytes_written += write(fd, temp_buffer, block_size)) < total_size) ;

	close(fd);
	sync();
}

static inline void io_bench_read() {
	int fd = open(temp_filename, O_RDONLY);
	size_t bytes_read = 0;
	
	while ((bytes_read += read(fd, temp_buffer, block_size)) < total_size) ;

	close(fd);
}

static inline void io_bench_write_mapped() {
	int fd = open(temp_filename, O_RDWR | O_TRUNC | O_CREAT, 0600);
	ftruncate(fd, total_size);

	void *mapped = mmap(NULL, total_size, PROT_WRITE, MAP_PRIVATE, fd, 0);

	size_t bytes_written = 0;
	
	while (bytes_written < total_size) {
		memcpy(mapped + bytes_written, temp_buffer, block_size);
		bytes_written += block_size;
	}

	munmap(mapped, total_size);

	close(fd);
	sync();
}

static inline void io_bench_read_mapped() {
	int fd = open(temp_filename, O_RDWR);

	void *mapped = mmap(NULL, total_size, PROT_READ, MAP_PRIVATE, fd, 0);

	size_t bytes_read = 0;
	
	while (bytes_read < total_size) {
		memcpy(temp_buffer, mapped + bytes_read, block_size);
		bytes_read += block_size;
	}

	munmap(mapped, total_size);

	close(fd);
}

void usage(const char *command) {
	fprintf(stderr, "Usage: %s -f <filename> [-b <block size (in KB)>] [-s <file size (in KB)]\n", command);
}

int main(int argc, char *argv[]) {
	int ch;

	while ((ch = getopt(argc, argv, "f:b:s:h?")) != -1) {
		switch (ch) {
			case 'f':
				temp_filename = optarg;
				break;

			case 'b':
				block_size = atoi(optarg) * 1024;
				break;

			case 's':
				total_size = atoi(optarg) * 1024;
				break;

			case '?':
			case 'h':
				usage(argv[0]);
				break;

			default:
				break;
		}
	}

	if (!temp_filename) {
		usage(argv[0]);
		return -1;
	}

	temp_buffer = malloc(block_size);

	BENCH_OP(io_bench_write);
	clear_temp_buffer_cache();
	BENCH_OP(io_bench_read);
	clear_temp_buffer_cache();
	BENCH_OP(io_bench_write_mapped);
	clear_temp_buffer_cache();
	BENCH_OP(io_bench_read_mapped);

	free(temp_buffer);
}

