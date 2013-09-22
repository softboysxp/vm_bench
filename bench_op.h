#ifndef BENCH_OP_H
#define BENCH_OP_H

#include <time.h>
#include <stdint.h>

#define BENCH_OP(func) { \
	struct timespec ts_start, ts_end; \
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts_start); \
	for (int i = 0; i < iter; i++) { \
		func(); \
	} \
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts_end); \
	uint64_t total_ns = (ts_end.tv_sec - ts_start.tv_sec) * 1000000000 + (ts_end.tv_nsec - ts_start.tv_nsec); \
	printf("%s: %.3f ns\n", #func, 1.0 * total_ns / iter); \
}

#endif //BENCH_OP_H

