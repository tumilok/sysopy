CC = gcc -Wall

all: main

main: main.c receiver.c packer.c sender.c common.c worker.c
		$(CC) -o main main.c common.c -lpthread -lrt
		$(CC) -o receiver receiver.c common.c worker.c -lpthread -lrt
		$(CC) -o packer packer.c common.c worker.c -lpthread -lrt
		$(CC) -o sender sender.c common.c worker.c -lpthread -lrt

test:
		make main
		./main

clean:
		rm -f main
		rm -f receiver
		rm -f packer
		rm -f sender