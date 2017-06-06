CPPFLAGS  =-std=c++11 -Wall -Wextra -g -pthread

all: MapReduceFramework.a Search 

MapReduceFramework.o: MapReduceFramework.cpp MapReduceFramework.h Thread.h ExecMapThread.h ExecReduceThread.h
	g++ $(CPPFLAGS) -c MapReduceFramework.cpp

MapReduceFramework.a: MapReduceFramework.o Thread.o ExecReduceThread.o ExecMapThread.o
	ar rcs MapReduceFramework.a MapReduceFramework.o Thread.o ExecReduceThread.o ExecMapThread.o

ExecReduceThread.o: ExecReduceThread.cpp ExecReduceThread.h MapReduceFramework.h Thread.h
	g++ $(CPPFLAGS) -c ExecReduceThread.cpp

ExecMapThread.o: ExecMapThread.cpp ExecMapThread.h Thread.h
	g++ $(CPPFLAGS) -c ExecMapThread.cpp

Thread.o: Thread.cpp Thread.h MapReduceClient.h
	g++ $(CPPFLAGS) -c Thread.cpp

Search: MapReduceFramework.a Search.cpp
	g++ $(CPPFLAGS) Search.cpp -L. MapReduceFramework.a -o Search

tar: MapReduceFramework.cpp Search.cpp
	tar -cvf ex3.tar $^ Makefile README

valgrind: Search
	valgrind --leak-check=full --show-possibly-lost=yes --show-reachable=yes --track-origins=yes --undef-value-errors=yes ./Search e /cs/usr/gullumbroso/Desktop/ /cs/usr/gullumbroso/Desktop/OS/ex2/project

gtest: MapReduceFramework.a test.cpp
	g++ $(CPPFLAGS) test.cpp -L. MapReduceFramework.a -o gtest

gvalgrind: gtest
	valgrind --leak-check=full --show-possibly-lost=yes --show-reachable=yes --track-origins=yes --undef-value-errors=yes ./gtest


clean:
	rm -f *.o *.a Search
