CC = gcc -Wall

all: main

main: main.c receiver.c packer.c sender.c
		$(CC) -o main main.c -lpthread -lrt
		$(CC) -o receiver receiver.c -lpthread -lrt
		$(CC) -o packer packer.c -lpthread -lrt
		$(CC) -o sender sender.c -lpthread -lrt

test:
		make main
		./main

clean:
		rm -f main
		rm -f receiver
		rm -f packer
		rm -f sender