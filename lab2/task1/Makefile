CC = gcc -Wall

compile:
		$(CC) main.c -o main
	

test:
		make clear
		make compile
		rm -f wyniki.txt

		for byte_num in 1 4 512 1024 4096 8192 ; do \
			for line_num in 1000 5000 10000; do \
				./main generate dane $$line_num $$byte_num ; \
				./main copy dane dane.$$line_num.$$byte_num.lib $$line_num $$byte_num lib ; \
				./main copy dane dane.$$line_num.$$byte_num.sys $$line_num $$byte_num sys ; \
			done; \
		done

		for byte_num in 1 4 512  1024  4096 8192 ; do \
			for line_num in 1000 5000 10000; do \
				./main sort dane.$$line_num.$$byte_num.lib $$line_num $$byte_num lib ; \
				./main sort dane.$$line_num.$$byte_num.sys $$line_num $$byte_num sys ; \
			done; \
		done

		make clear

clear:
		rm -f *.o *.a *.so dane* main