#!/bin/bash

LOG=all.log
CSV=all.csv
ITER=$1

rm -f $LOG
touch $LOG

for I in `seq 1 $ITER`; do
	echo -e '\E[37;44m'"\033[1mRun $I/$ITER\033[0m"
	./syscall_bench | tee -a $LOG
	./vm_bench | tee -a $LOG
	./io_bench -f `mktemp` | tee -a $LOG
	./pthreads_bench | tee -a $LOG
	./ipc_bench | tee -a $LOG
done

cat $LOG | awk -F '[ :]' -v OFS=',' '{print $1,$3}' > $CSV

