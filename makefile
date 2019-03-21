CC=gcc
CFLAGS=-O3 -Wall -Wextra -pedantic
LFLAGS=

OBJS=
BINS=eggtimer

.PHONY: clean all

all: $(BINS) $(OBJS)

clean:
	rm -f $(BINS) $(OBJS) *.o

%.o: %.c
	$(CC) $(CFLAGS) $(LFLAGS) -c -o $@ $<

$(BINS): % : %.o $(OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $@.o $(OBJS)
