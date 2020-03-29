$(CC) = gcc -Wall

all:
		make clean
		make compile
		make test
		make clean

compile:
		$(CC) inheritance.c -o inheritance
		$(CC) child.c -o child

test:
		./inheritance ignore
		./inheritance handler
		./inheritance mask
		./inheritance pending

clean:
		rm -f inheritance child