
# gcc -I ../gopts/include -o test test.c ../gopts/libgopts.a

PROGRAM = test
CFLAGS = -g -Wall -I ../include
LIB = ../lib/libnanonet.a

$(PROGRAM): src/test.c
	$(CC) $(CFLAGS) -o $@ $< $(LIB)

clean:
	rm test

