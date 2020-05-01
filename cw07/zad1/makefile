CC = gcc -Wall

all: main

main: main.c receiver.c packer.c sender.c common.c worker.c
		$(CC) -o main common.c main.c
		$(CC) -o receiver common.c worker.c receiver.c
		$(CC) -o packer common.c worker.c packer.c
		$(CC) -o sender common.c worker.c sender.c

test:
		make main
		./main

clean:
		rm -f main
		rm -f receiver
		rm -f packer
		rm -f sender