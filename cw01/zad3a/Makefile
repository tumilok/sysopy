CC = gcc -Wall

static:
		$(CC) -c comparison.c
		$(CC) -c test.c
		ar crs libcomparison.a comparison.o test.o
		$(CC) -o main main.c -L. -lcomparison

shared:
		$(CC) -c -fPIC comparison.c
		$(CC) -c -fPIC test.c
		$(CC) -shared -fPIC -o libcomparison.so comparison.o test.o
		$(CC) -o main main.c -L. -Wl,-rpath=. -lcomparison

dynamic:
		$(CC) -c -fPIC comparison.c
		$(CC) -shared -fPIC -o libcomparison.so comparison.o
		$(CC) -o main dyno_main.c -Wl,--no-as-needed -ldl -I .

exec:
		./main

clear:
		rm -f *.o *.a *.so tmp.txt main

test:
		echo -e "aaa\nbbb ccc\nddd\neee\nhhh iii\n" > a.txt
		echo -e "jjj kkk\naaa\nfff ccc\neee\nbbb ggg" > b.txt
		echo -e "bbbbbbbbb\naaaaaaaaa\nccddccdddcc\ndd agga ad\nabcderghrshabcd \nabcagsegerghsdh\nfhsrth" > c.txt
		echo -e "aaaaaaaaa\nbbbbbbbbb\nccccccccc\ndddddd dd\nabcdabcd \nabcd" > d.txt
		./main compare_pair a.txt:b.txt remove_block 0   
		./main compare_pair a.txt:b.txt remove_block 0 compare_pair a.txt:b.txt remove_block 1