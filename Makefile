CC = g++
CFLAGS = -g
OFLAGS = -lpthread

all: clean prefixsum
	  
prefixsum: prefixSum.c
	@$(CC) $(CFLAGS) $(OFLAGS) prefixSum.c -o prefixSum
	@$(CC) $(CFLAGS) $(OFLAGS) sprefixSum.c -o sprefixSum

clean: prefixSum.o sprefixSum.o
	@rm prefixSum.o sprefixSum.o
