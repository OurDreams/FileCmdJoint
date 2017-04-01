CFLAGS = -Wall -g
SRC=$(wildcard src/*.c)
HEADERS=$(wildcard src/*.h)
FileCmdJoing: $(HEADERS) $(SRC) ; gcc -o $@ $^ $(CFLAGS)