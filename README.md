# MultithreadedServer

This is a multi server simulation project tested with this below Make file
all: par_sum.c
	gcc -g -O0 --std=c99 -Wall -o ops par_sum.c 
	
clean:
	rm -f ops
