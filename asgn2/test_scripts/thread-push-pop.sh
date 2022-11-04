
cleanup(){
	rm -r test_dir
}

QUEUE="queue.o queue.h"
TEST_FILES="test_files/thread-push-pop.c test_files/Makefile"

mkdir test_dir

cp $QUEUE test_dir
cp  $TEST_FILES test_dir

make -C test_dir

./test_dir/queue-test

if [ $? != 0 ]
then
	cleanup
	exit 1
fi

cleanup
