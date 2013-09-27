CC = clang
CFLAGS = -std=c99 -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lrt -lpthread

ifdef DEBUG
	CFLAGS += -DDEBUG -O0 -g
else
	CFLAGS += -O3
endif

BINS = syscall_bench vm_bench io_bench pthreads_bench ipc_bench

all: $(BINS)

syscall_bench: syscall_bench.c bench_op.h
vm_bench: vm_bench.c bench_op.h
io_bench: io_bench.c bench_op.h
pthreads_bench: pthreads_bench.c bench_op.h
ipc_bench: ipc_bench.c bench_op.h

$(BINS):
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

run: $(BINS)
	./syscall_bench

clean:
	rm -f $(BINS)

zip: archive

archive:
	git archive --prefix=vm_bench/ --format zip -9 HEAD > vm_bench-`git log -1 --format='%h'`.zip

