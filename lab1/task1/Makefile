static:
		make clear
		gcc $(GFLAGS) -c comparison.c
		ar crs libcomparison.a comparison.o
		gcc $(GFLAGS) -o main main.c -L. -lcomparison

dynamic:
		make clear
		gcc $(GFLAGS) -c comparison.c
		gcc $(GFLAGS) -fPIC -shared comparison.c -o libcomparison.so
		gcc $(GFLAGS) -o main main.c -L. -Wl,-rpath=. -lcomparison

exec:
		./main

clear:
		rm -f *.o *.a *.so main