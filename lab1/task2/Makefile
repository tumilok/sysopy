compile:
		make clear
		gcc $(GFLAGS) -c comparison.c
		gcc $(GFLAGS) -c test.c
		ar crs libcomparison.a comparison.o test.o
		gcc $(GFLAGS) -o main main.c -L. -lcomparison

exec:
		./main

clear:
		rm -f *.o *.a *.so main