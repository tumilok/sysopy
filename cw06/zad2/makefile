$(CC) = gcc -Wall

all: client.c server.c
	$(CC) -o client client.c -lrt
	$(CC) -o server server.c -lrt

clean:
	rm -f server client
