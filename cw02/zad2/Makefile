CC = gcc -Wall

compile:
	$(CC) find_nftw.c find_stat.c utils.c main.c -o main

clean:
	rm -f main

test_stat:
	mkdir dir1
	mkdir dir1/dir2
	mkdir dir1/dir2/dir3
	touch dir1/file1
	touch dir1/dir2/file2
	touch dir1/dir2/dir3/file3
	touch dir1/file12
	ln -s dir1 link1
	ln -s dir1/dir2 link2

	make compile

	./main ../ atime -30 maxdepth 10 stat

	make clean_all

test_nftw:
	mkdir dir1
	mkdir dir1/dir2
	mkdir dir1/dir2/dir3
	touch dir1/file1
	touch dir1/dir2/file2
	touch dir1/dir2/dir3/file3
	touch dir1/file12
	ln -s dir1 link1
	ln -s dir1/dir2 link2

	make compile

	./main ../ mtime -30 maxdepth 10 nftw

	make clean_all

clean_all:
	make clean
	rm -f file1
	rm -f file2
	rm -f file3
	rm -f link1
	rm -f link2
	rm -r dir1