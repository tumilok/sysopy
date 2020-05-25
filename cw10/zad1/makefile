CC = gcc -Wall

compile: server.c client.c common.c
		$(CC) server.c common.c -o server -lpthread -lm
		$(CC) client.c common.c -o client -lpthread -lm

clean:
		rm -f server client