
cleanup(){
	rm -r test_dir
}

QUEUE="queue.o queue.h"
TEST_FILES="test_files/create-free.c test_files/Makefile"

mkdir test_dir

cp $QUEUE test_dir
cp  $TEST_FILES test_dir

make -C test_dir

# run once to check for being able to run
./test_dir/queue-test

if [ $? != 0 ]
then
	cleanup
	exit 1
fi

# run again for memory leaks
valgrind ./test_dir/queue-test >/dev/null 2>/dev/null

if [ $? != 0 ]
then
	cleanup
	exit 1
fi
cleanup
