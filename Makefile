CC = g++
CFLAGS = -g -std=c++11 -c -Wvla -Wall -Wextra
CODEFILES = Makefile README uthreads.cpp Thread.h Thread.cpp
VFLAGS = valgrind -v --show-possibly-lost=yes --show-reachable=yes --undef-value-errors=yes


all: libuthreads


libuthreads: uthreads.o uthreads.h Thread.o Thread.h
	ar rcs libuthreads.a uthreads.o Thread.o 

Thread.o: Thread.cpp Thread.h
	$(CC) $(CFLAGS) Thread.cpp -o Thread.o

uthreads.o: uthreads.cpp uthreads.h Thread.h
	$(CC) $(CFLAGS) uthreads.cpp -o uthreads.o

test.o: test.cpp uthreads.h
	$(CC) $(CFLAGS) test.cpp -o test.o

Threads: test.o uthreads.o Thread.o
	$(CC) test.o uthreads.o Thread.o -o Threads

run_val: Threads
	$(VFLAGS) --log-file="valgrin.out" Threads

tar:
	tar -cf ex2.tar $(CODEFILES)

clean:
	-rm -f *.o libuthreads.a Threads valgrind.out
