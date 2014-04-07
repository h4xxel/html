CFLAGS	+= -Wall --std=c99 -g
LDFLAGS	+=

BIN	= html
SRC	= $(wildcard *.c)
OBJ	= $(SRC:.c=.o)

.PHONY: all clean install

all: $(OBJ)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)

clean:
	rm -f $(BIN) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
