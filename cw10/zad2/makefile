CC = gcc -Wall

run_server: server
		./server 8888 .socket

run_local_client: client
		./client mike local .socket

run_inet_client: client
		./client josh inet 127.0.0.1 8888

compile: server.c client.c common.c
		$(CC) server.c common.c -o server -lpthread -lm
		$(CC) client.c common.c -o client -lpthread -lm

clean:
		rm -f server client