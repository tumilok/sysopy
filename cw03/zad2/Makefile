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

testing:
		rm -f c.txt
		make clean
		make compile
		./main lista 3 3 -shared
		make clean