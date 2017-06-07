CPPFLAGS  =-std=c++11 -Wall -Wextra -g

all: CacheFS.a TEST

CacheFS.o: CacheFS.cpp CacheFS.h FileDesc.h CacheBlock.h LRUCache.h LFUCache.h FBRCache.h
	g++ $(CPPFLAGS) -c CacheFS.cpp
 
CacheFS.a: CacheFS.o FileDesc.o CacheBlock.o LRUCache.o LFUCache.o FBRCache.o Cache.o
	ar rcs CacheFS.a CacheFS.o FileDesc.o CacheBlock.o LRUCache.o LFUCache.o FBRCache.o Cache.o

FileDesc.o: FileDesc.cpp FileDesc.h
	g++ $(CPPFLAGS) -c FileDesc.cpp

LRUCache.o: LRUCache.cpp LRUCache.h Cache.h CacheBlock.h
	g++ $(CPPFLAGS) -c LRUCache.cpp

LFUCache.o: LFUCache.cpp LFUCache.h Cache.h CacheBlock.h
	g++ $(CPPFLAGS) -c LFUCache.cpp

FBRCache.o: FBRCache.cpp FBRCache.h Cache.h CacheBlock.h
	g++ $(CPPFLAGS) -c FBRCache.cpp

Cache.o: Cache.cpp Cache.h
	g++ $(CPPFLAGS) -c Cache.cpp

CacheBlock.o: CacheBlock.cpp CacheBlock.h
	g++ $(CPPFLAGS) -c CacheBlock.cpp

TEST: CacheFS.a TEST.cpp
	g++ $(CPPFLAGS) TEST.cpp -L. CacheFS.a -o TEST

tar: CacheFS.cpp
	tar -cvf ex4.tar $^ Makefile README

valgrind: TEST
	valgrind --leak-check=full --show-possibly-lost=yes --show-reachable=yes --track-origins=yes --undef-value-errors=yes ./TEST

gtest: CacheFS.a TEST.cpp
	g++ $(CPPFLAGS) TEST.cpp -L. CacheFS.a -o gtest

gvalgrind: gtest
	valgrind --leak-check=full --show-possibly-lost=yes --show-reachable=yes --track-origins=yes --undef-value-errors=yes ./gtest


clean:
	rm -f *.o *.a TEST
