
cleanup(){
	rm -r test_dir
	rm $syscalls
	rm $times
}

QUEUE="queue.o queue.h"
TEST_FILES="test_files/pop-busy.c test_files/Makefile"
DATE="date +%s.%N"

syscalls=strace.out
times=time.out

mkdir test_dir

cp $QUEUE test_dir
cp $TEST_FILES test_dir

make -C test_dir

(sleep 1;date +%s.%N;sleep 1;date +%s.%N) | strace -ttt -T -f -o $syscalls ./test_dir/queue-test > $times

if [ $? != 0 ]
then
	echo "bad exit"
	cleanup
	exit 1
fi

start=`head -n 1 $times`
end=`tail -n 1 $times`

active=`test_files/rainsford.py -s $start -e $end -t 3 $syscalls`

nothreadrun=`echo "$active" | awk -F':' '{if ($1 < 1) sum+=$2;} END{print int(sum * 100);}'`

if [ $nothreadrun -lt 90 ]
then
	echo $nothreadrun "is not the right amount"
	cleanup
	exit 1
fi

cleanup
