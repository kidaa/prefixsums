CC = g++
CFLAGS = -g
OFLAGS = -lpthread

all: prefixsum
	  
prefixsum: prefixSum.c
	$(CC) $(CFLAGS) $(OFLAGS) prefixSum.c -o prefixSum
	$(CC) $(CFLAGS) $(OFLAGS) sprefixSum.c -o sprefixSum
