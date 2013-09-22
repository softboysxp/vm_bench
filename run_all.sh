#!/bin/bash

LOG_DIR=results/`hostname -s`
CSV=all.csv
ITER=$1

rm -rf $LOG_DIR
mkdir -p $LOG_DIR

for I in `seq 1 $ITER`; do
	echo -e '\E[37;44m'"\033[1mRun $I/$ITER\033[0m"
	./syscall_bench -i 10000 | tee -a $LOG_DIR/syscall_bench.log
	./vm_bench -i 10000 | tee -a $LOG_DIR/vm_bench.log
	./io_bench -f `mktemp -u` | tee -a $LOG_DIR/io_bench.log
	if [ -b "/dev/xvdb" ]; then
		./io_bench -f /mnt/io_bench.local | tee -a $LOG_DIR/io_bench_local.log
		rm -f /mnt/io_bench.local
	fi
	./pthreads_bench -i 10000 | tee -a $LOG_DIR/pthreads_bench.lgo
	./ipc_bench -i 10000 | tee -a $LOG_DIR/ipc_bench.log
done

for LOG in $LOG_DIR/*.log; do
	cat $LOG | awk -F '[ :]' -v OFS=',' '{print $1,$3}' > $LOG_DIR/`basename $LOG .log`.csv
done
