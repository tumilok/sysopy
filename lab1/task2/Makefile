CC = gcc -Wall

compile:
		$(CC) -c comparison.c
		$(CC) -c test.c
		ar crs libcomparison.a comparison.o test.o
		$(CC) -o main main.c -L. -lcomparison

exec:
		./main

clear:
		rm -f *.o *.a *.so tmp.txt main

test:
		./main compare_pair a.txt:b.txt c.txt:d.txt e.txt:f.txt remove_block 0 remove_operation 1 0