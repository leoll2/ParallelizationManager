CC = g++
CFLAGS = -Wall
PTHREAD = -lpthread

# Uncomment the next first OR second line if you want the manager to be verbose OR quiet, respectively
#DEBUG = -DDEBUG
DEBUG = 

BIN = bin
BUILD = build
SRC = src
TEST = test

.PHONY: all clean

all: $(BIN)/test1 $(BIN)/test2 $(BIN)/test3

################
# Object files #
################


$(BUILD)/%.o: $(SRC)/%.cpp
	$(CC) $(CFLAGS) $(DEBUG) -c -o $@ $<
	
$(BUILD)/%.o: $(TEST)/%.cpp
	mkdir -p build
	$(CC) $(CFLAGS) $(DEBUG) -I$(SRC) -c -o $@ $<


################
# Executables  #
################

$(BIN)/test%: $(BUILD)/test%.o $(BUILD)/parallelizer.o $(BUILD)/activity.o $(BUILD)/semaphore.o $(BUILD)/task.o
	mkdir -p bin
	$(CC) -o $@ $^ $(PTHREAD)


clean:
	rm -f $(BUILD)/*.o $(BIN)/test*
	rmdir bin build
