CC=gcc
CFLAGS=-O3 -Wall -Wextra -pedantic
LFLAGS=

BIN_DIR=bin
OBJ_DIR=obj
SRC_DIR=src

BIN_NAMES=eggtimer
OBJ_NAMES=

BINS=$(foreach I, $(BIN_NAMES), $(BIN_DIR)/$I)
OBJS=$(foreach I, $(OBJ_NAMES), $(OBJ_DIR)/$I.o)


.PHONY: clean all install remove

all: $(BINS) $(OBJS)

clean:
	rm -f $(BIN_DIR)/* $(OBJ_DIR)/*

install: $(BINS)
	cp $(BINS) /bin/

remove:
	rm $(foreach I, $(BIN_NAMES), /bin/$I)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(LFLAGS) -c -o $@ $<

$(BIN_DIR)/%: $(OBJ_DIR)/%.o $(OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $^
