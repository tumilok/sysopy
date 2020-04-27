CC = gcc
FLAGS = -Wall

all: main

main: main.c worker1.c worker2.c worker3.c
	$(CC) $(FLAGS) -o main common.c main.c
	$(CC) $(FLAGS) -o worker1 common.c worker1.c
	$(CC) $(FLAGS) -o worker2 common.c worker2.c
	$(CC) $(FLAGS) -o worker3 common.c worker3.c


test:
	make main
	./main

clean:
	rm -f main
	rm -f worker1
	rm -f worker2
	rm -f worker3