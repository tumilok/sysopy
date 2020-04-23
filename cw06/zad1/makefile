$(CC) = gcc -Wall

all:
		make compile
		make clean

compile:
		$(CC) server.c -o server
		$(CC) client.c -o client

clean:
		rm -f server client