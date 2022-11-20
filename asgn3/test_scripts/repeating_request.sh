#!/usr/bin/env bash

port=$(bash test_files/get_port.sh)

# get a random port to connect to
./httpserver $port > /dev/null &
pid=$!

# Wait until we can connect.
while ! nc -zv localhost $port; do
    sleep 0.01
done

# for 5 times
for i in {1..5}; do
    expected=400
    
    echo -c 2048 test_files/wonderful.txt > requestfile
    printf "GET GET GET" >> requestfile

    # compare the return(status code)
    actual=$(cat requestfile | test_files/my_nc.py localhost $port | head -n 1 | awk '{ print $2 }')

    # Check the status code.
    if [[ $actual -ne $expected ]]; then
        # Kill the server
        kill -9 $pid
        wait $pid
        exit 1
    fi
done

# kill the server
kill -9 $pid


exit 0
