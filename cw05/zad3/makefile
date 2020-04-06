$(CC) = gcc -Wall

all:
		make clean
		make compile
		make exec

compile:
		$(CC) producer.c -o producer
		$(CC) consumer.c -o consumer
		$(CC) main.c -o main
		mkdir files
		touch files/result.txt

exec:
		./main

clean:
		rm -rf pipe producer consumer main files
