CC = gcc -Wall
INPUT_FILE = mona_lisa.ascii.pgm


compile: main.c
	$(CC) -o main main.c -lpthread -lm

clean:
	rm -f main Times.txt
	rm -rf res

test:
	make clean
	make compile
	mkdir res
	./main 1 sign $(INPUT_FILE) res/sign1.txt >> Times.txt
	./main 2 sign $(INPUT_FILE) res/sign2.txt >> Times.txt
	./main 4 sign $(INPUT_FILE) res/sign4.txt >> Times.txt
	./main 8 sign $(INPUT_FILE) res/sign8.txt >> Times.txt

	./main 1 block $(INPUT_FILE) res/block1.txt >> Times.txt
	./main 2 block $(INPUT_FILE) res/block2.txt >> Times.txt
	./main 4 block $(INPUT_FILE) res/block4.txt >> Times.txt
	./main 8 block $(INPUT_FILE) res/block8.txt >> Times.txt

	./main 1 interleaved $(INPUT_FILE) res/interleaved1.txt >> Times.txt
	./main 2 interleaved $(INPUT_FILE) res/interleaved2.txt >> Times.txt
	./main 4 interleaved $(INPUT_FILE) res/interleaved4.txt >> Times.txt
	./main 8 interleaved $(INPUT_FILE) res/interleaved8.txt >> Times.txt
