#!/usr/bin/env bash

port=$(bash test_files/get_port.sh)

# Start up server.
./httpserver $port > /dev/null &
pid=$!

# Wait until we can connect.
while ! nc -zv localhost $port; do
    sleep 0.01
done

for i in {1..5}; do
    # just test a simple file that does not exist
    file="whatisit.txt"
    printf "simple" >>file

    # Expected status code.
    expected=404

    # The only thing that is should be printed is the status code.
    actual=$(curl -s -w "%{http_code}" -o /dev/null localhost:$port/$file)

    # Check the status code.
    if [[ $actual -ne $expected ]]; then
        # Make sure the server is dead.
        kill -9 $pid
        wait $pid
        rm -rf $dir
        exit 1
    fi

    # In case they create the file.
    rm -rf $dir
done

# Make sure the server is dead.
kill -9 $pid

# In case they create the file.
rm -rf $dir

exit 0
