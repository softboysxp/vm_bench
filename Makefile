CC = clang
CFLAGS = -std=c99 -D_BSD_SOURCE
LDFLAGS = -lpthread

ifeq "$(shell uname)" "Linux"
	LDFLAGS += -lrt
	CFLAGS += -D_POSIX_C_SOURCE=200809L
endif

ifdef DEBUG
	CFLAGS += -DDEBUG -O0 -g
else
	CFLAGS += -O3 -Wall
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
	rm -f *.zip
	rm -rf *.dSYM

zip: archive

archive:
	git archive --prefix=vm_bench/ --format zip -9 HEAD > vm_bench-`git log -1 --format='%h'`.zip

