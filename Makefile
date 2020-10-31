CFLAGS = -std=c11 -g -static

kucc: kucc.c

test: kucc
	./test.sh

clean:
	rm -f kucc *.o *~ tmp*

.PHONY: test clean
