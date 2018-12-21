CC = g++
CFLAGS = -Wall
#DEBUG = -DDEBUG
DEBUG = 
PTHREAD = -lpthread
SRC = src
BUILD = build

.PHONY: all clean

all: main

################
# Object files #
################

$(BUILD)/activity.o: $(SRC)/activity.cpp
	$(CC) $(CFLAGS) $(DEBUG) -c $(SRC)/activity.cpp -o $(BUILD)/activity.o

$(BUILD)/main.o: $(SRC)/main.cpp
	$(CC) $(CFLAGS) $(DEBUG) -c $(SRC)/main.cpp -o $(BUILD)/main.o

$(BUILD)/parallelizer.o: $(SRC)/parallelizer.cpp
	$(CC) $(CFLAGS) $(DEBUG) -c $(SRC)/parallelizer.cpp -o $(BUILD)/parallelizer.o

$(BUILD)/semaphore.o: $(SRC)/semaphore.cpp
	$(CC) $(CFLAGS) $(DEBUG) -c $(SRC)/semaphore.cpp -o $(BUILD)/semaphore.o

$(BUILD)/task.o: $(SRC)/task.cpp
	$(CC) $(CFLAGS) $(DEBUG) -c $(SRC)/task.cpp -o $(BUILD)/task.o


################
# Executables  #
################

main: $(BUILD)/main.o $(BUILD)/parallelizer.o $(BUILD)/activity.o $(BUILD)/semaphore.o $(BUILD)/task.o
	$(CC) $(PTHREAD) $(BUILD)/main.o $(BUILD)/parallelizer.o $(BUILD)/activity.o $(BUILD)/semaphore.o $(BUILD)/task.o -o main


clean:
	rm -f $(BUILD)/*.o main
