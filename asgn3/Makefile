CC=gcc
CFLAGS = -Wall -Werror -Wextra -pedantic

httpserver: httpserver.o httpserver.c bind.c
	$(CC) -o httpserver httpserver.c bind.c
	
clean:
	rm httpserver
