CC = gcc -Wall

static:
		$(CC) -c comparison.c
		ar crs libcomparison.a comparison.o
		$(CC) -o main main.c -L. -lcomparison

shared:
		$(CC) -c -fPIC comparison.c
		$(CC) -shared -fPIC -o libcomparison.so comparison.o
		$(CC) -o main main.c -L. -Wl,-rpath=. -lcomparison

exec:
		./main

clear:
		rm -f *.o *.a *.so tmp.txt main