.SUFFIXES: .c

CC      = gcc
PROGRAM = ../lib/libnanonet.a
CFLAGS  = -g -Wall -I ../include
FILES   = net.o state.o log.o buffer.o
OBJS    = $(FILES:%=../lib/%)
SRCS    = $(OBJS:%.o=%.c)

${PROGRAM}:$(OBJS)
	ar rv $@ $?
	ranlib $@

../lib/%.o:%.c
	$(CC) $(CFLAGS) -o ../lib/$*.o -c $<

clean:
	rm -f $(OBJS)
	rm -f $(PROGRAM)



