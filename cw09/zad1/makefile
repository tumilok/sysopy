CC = gcc
FLAGS = -Wall
CHAIRS_NUMBER = 2
CLIENTS_NUMBER = 4

compile: main.c
	$(CC) $(FLAGS) -o main main.c -lpthread -lm

clean:
	rm -f main

test:
	make clean
	make compile
	./main $(CHAIRS_NUMBER) $(CLIENTS_NUMBER)