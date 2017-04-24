CC = g++
CFLAGS = -g -std=c++11 -c -Wvla -Wall -Wextra
CODEFILES = Makefile README uthreads.h uthreads.cpp Thread.h Thread.cpp ThreadManager.h ThreadManager.cpp
VFLAGS = valgrind -v --show-possibly-lost=yes --show-reachable=yes --undef-value-errors=yes


all: libuthreads


libuthreads: uthreads.o uthreads.h Thread.o Thread.h ThreadManager.o ThreadManager.h
	ar rcs libuthreads.a uthreads.o Thread.o ThreadManager.o

Thread.o: Thread.cpp Thread.h
	$(CC) $(CFLAGS) Thread.cpp -o Thread.o

ThreadManager.o: ThreadManager.cpp ThreadManager.h Thread.h
	$(CC) $(CFLAGS) ThreadManager.cpp -o ThreadManager.o

uthreads.o: uthreads.cpp uthreads.h ThreadManager.h Thread.h
	$(CC) $(CFLAGS) uthreads.cpp -o uthreads.o

main.o: main.cpp uthreads.h
	$(CC) $(CFLAGS) main.cpp -o main.o


Threads: main.o uthreads.o ThreadManager.o Thread.o
	$(CC) main.o uthreads.o ThreadManager.o Thread.o -o Uthreads


run_val: Threads
	$(VFLAGS) --log-file="valgrin.out" Threads

tar:
	tar -cf ex2.tar $(CODEFILES)

clean:
	-rm -f *.o libuthreads.a Threads valgrind.out
