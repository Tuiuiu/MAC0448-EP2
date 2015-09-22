BIN := servidor

CFLAGS = -ansi -Wall -pedantic -std=c++11
LDFLAGS = -lpthread

CC = g++

SRC := $(filter-out $(BIN).cpp,$(wildcard *.cpp))
OBJ := $(SRC:.cpp=.o)

$(BIN): $(OBJ) $(BIN).o
	$(CC) $^ -o $@ $(LDFLAGS)

$(BIN).o: %.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ): %.o: %.cpp %.hpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o $(BIN)
