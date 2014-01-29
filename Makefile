CFLAGS	+= -Wall --std=c99 -g

.PHONY: all clean install
all:
	$(CC) $(CFLAGS) -o html *.c $(LDFLAGS)

clean:
	rm -f html
