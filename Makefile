CC = g++
CFLAGS = -Wall
DEBUG = -DDEBUG


.PHONY: all clean

all: main

################
# Object files #
################

main.o: main.cpp
	$(CC) $(CFLAGS) $(DEBUG) -c main.cpp -o main.o

manager.o: manager.cpp
	$(CC) $(CFLAGS) $(DEBUG) -c manager.cpp -o manager.o


################
# Executables  #
################

main: main.o manager.o
	$(CC) main.o manager.o -o main


clean:
	rm *.o main
