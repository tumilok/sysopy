$(CC) = gcc -Wall

compile_all:
		make compile_main
		make compile_worker
		make compile_test

compile_main:
		$(CC) ../src/matrix.c ../src/main.c -o main

compile_worker:
		$(CC) ../src/matrix.c ../src/worker.c -o worker

compile_test:
		$(CC) ../src/matrix.c test.c -o test

clean:
		rm -f main worker test *.txt

run_tester:
		make clean
		make compile_all
		./test 10 4 9
		make clean
