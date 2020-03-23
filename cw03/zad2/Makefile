$(CC) = gcc -Wall

compile:
		$(CC) main.c -o main

clean:
		rm -f main

test:
		make compile
		./main lista 5 10 shared
		make clean