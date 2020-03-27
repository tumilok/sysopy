$(CC) = gcc -Wall

compile:
	make main
	make worker
	make test

main:
		$(CC) matrix.c main.c -o main

worker:
		$(CC) matrix.c worker.c -o worker

test:
		$(CC) matrix.c test.c -o test

clean:
		rm -f main worker test

test_separated:
		rm -f c.txt
		make clean
		make compile
		./main lista 3 3 -separated
		make clean

test_shared:
		rm -f c.txt
		make clean
		make compile
		./main lista 3 3 -shared
		make clean

test_all:
		make clean
		make compile
		./test 10 4 9
