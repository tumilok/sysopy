CC = gcc -Wall

compile:
		$(CC) cproc.c -o cproc

clean:
		rm -f cproc
		rm -f file1
		rm -f file2
		rm -f file3
		rm -f link1
		rm -f link2
		rm -r dir1

create_files:
		mkdir dir1
		mkdir dir1/dir2
		mkdir dir1/dir2/dir3
		touch dir1/file1
		touch dir1/dir2/file2
		touch dir1/dir2/dir3/file3
		touch dir1/file12
		ln -s dir1 link1
		ln -s dir1/dir2 link2

test_stat:
		make create_files
		make compile
		./cproc ../ stat
		make clean

test_nftw:
		make create_files
		make compile
		./cproc ../ nftw
		make clean

test_all:
		make test_stat
		make test_nftw
