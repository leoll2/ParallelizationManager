CC = g++
CFLAGS = -Wall
#DEBUG = -DDEBUG
DEBUG = 
PTHREAD = -lpthread

BIN = bin
BUILD = build
SRC = src
TEST = test

.PHONY: all clean

all: test1 test2

################
# Object files #
################


$(BUILD)/%.o: $(SRC)/%.cpp
	$(CC) $(CFLAGS) $(DEBUG) -c -o $@ $<
	
$(BUILD)/%.o: $(TEST)/%.cpp
	$(CC) $(CFLAGS) $(DEBUG) -c -o $@ $<


################
# Executables  #
################

test%: $(BUILD)/test%.o $(BUILD)/parallelizer.o $(BUILD)/activity.o $(BUILD)/semaphore.o $(BUILD)/task.o
	$(CC) -o $(BIN)/$@ $^ $(PTHREAD)


clean:
	rm -f $(BUILD)/*.o $(BIN)/test*
