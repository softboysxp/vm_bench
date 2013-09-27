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

#ifndef BENCH_OP_H
#define BENCH_OP_H

#include <stdint.h>

#ifdef __APPLE__

#include <mach/mach_time.h>

#define BENCH_OP(func) { \
	mach_timebase_info_data_t timebase_info; \
	mach_timebase_info(&timebase_info); \
	uint64_t start, end; \
	start = mach_absolute_time(); \
	for (int i = 0; i < iter; i++) { \
		func(); \
	} \
	end = mach_absolute_time(); \
	uint64_t total_ns = (end - start) * timebase_info.numer / timebase_info.denom; \
	printf("%s: %.3f ns\n", #func, 1.0 * total_ns / iter); \
	fflush(stdout); \
}

#else

#include <time.h>

#ifndef CLOCK_MONOTONIC_RAW
	#define CLOCK_MONOTONIC_RAW CLOCK_MONOTONIC
#endif

#define BENCH_OP(func) { \
	struct timespec ts_start, ts_end; \
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts_start); \
	for (int i = 0; i < iter; i++) { \
		func(); \
	} \
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts_end); \
	uint64_t total_ns = (ts_end.tv_sec - ts_start.tv_sec) * 1000000000 + (ts_end.tv_nsec - ts_start.tv_nsec); \
	printf("%s: %.3f ns\n", #func, 1.0 * total_ns / iter); \
	fflush(stdout); \
}

#endif //__APPLE__

#endif //BENCH_OP_H

