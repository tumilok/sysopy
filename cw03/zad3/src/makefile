$(CC) = gcc -Wall

compile:
		make main
		make worker

main:
		$(CC) matrix.c main.c -o main

worker:
		$(CC) matrix.c worker.c -o worker

clean:
		rm -f main worker

clean_all:
		make clean
		rm -f c.txt

test_separated:
		make clean_all
		make compile
		./main lista 3 7 -separated 3 5
		make clean

test_shared:
		rm -f c.txt
		make clean
		make compile
		./main lista 3 7 -shared 3 5
		make clean

test_all:
		make test_shared
		make test_separated
