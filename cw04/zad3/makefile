$(CC) = gcc -Wall

run:
		make clean
		make compile
		make test_segfault
		make test_child
		make test_floatex
		make clean

compile:
		$(CC) sigtest.c -o sigtest

test_segfault:
		./sigtest segfault

test_child:
		./sigtest child

test_floatex:
		./sigtest floatex

clean:
		rm -f sigtest