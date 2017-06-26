CPPFLAGS  =-std=c++11 -Wall -Wextra -g -lboost_regex

all: whatsappServer whatsappClient

whatsappServer: whatsappServer.o 
	g++ $(CPPFLAGS) whatsappServer.o -o whatsappServer


whatsappServer.o: whatsappServer.cpp whatsappServer.h
	g++ $(CPPFLAGS) -c whatsappServer.cpp -o whatsappServer.o

whatsappClient: whatsappClient.o
	g++ $(CPPFLAGS) whatsappClient.o -o whatsappClient

whatsappClient.o: whatsappClient.cpp whatsappClient.h
	g++ $(CPPFLAGS) -c whatsappClient.cpp -o whatsappClient.o

valgrind: Tester
	valgrind --leak-check=full --show-possibly-lost=yes --show-reachable=yes --track-origins=yes --undef-value-errors=yes ./Tester

clean:
	rm -f *.o *.a Tester
