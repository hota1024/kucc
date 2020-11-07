CFLAGS = -std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

kucc: $(OBJS)
	$(CC) -o kucc $(OBJS) $(LDFLAGS)

test: kucc
	./test.sh

clean:
	rm -f kucc *.o *~ tmp*

.PHONY: test clean
