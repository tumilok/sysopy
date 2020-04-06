$(CC) = gcc -Wall

all:
		make clean
		make compile
		make test
		make clean

compile:
		$(CC) sort.c -o sort

clean:
		rm -f sort

test:
		sudo ./sort test.txt
