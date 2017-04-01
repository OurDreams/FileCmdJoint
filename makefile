CFLAGS = -Wall -g
SRC=$(wildcard src/*.c)
HEADERS=$(wildcard src/*.h)
FileCmdJoint: $(HEADERS) $(SRC) ; gcc -o $@ $^ $(CFLAGS)