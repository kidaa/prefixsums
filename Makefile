CC = gcc
CFLAGS = -g
OFLAGS = -lpthread

all: prefixsum
	  
prefixsum: prefixSum.c
	$(CC) $(CFLAGS) $(OFLAGS) -std=c99 prefixSum.c -o prefixSum
	$(CC) $(CFLAGS) $(OFLAGS) -std=c99 sprefixSum.c -o sprefixSum
