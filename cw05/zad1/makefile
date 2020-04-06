$(CC) = gcc -Wall

run_all:
		make clean
		make compile
		make test
		make clean

compile:
		$(CC) interpreter.c -o interpreter

clean:
		rm -f interpreter

test:
		./interpreter commands.txt