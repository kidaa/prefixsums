CC = g++
CFLAGS = -g
OFLAGS = -lpthread

all: clean prefixsum
	  
prefixsum: prefixSum.cpp
	@$(CC) $(CFLAGS) $(OFLAGS) prefixSum.cpp -o prefixSum
	@$(CC) $(CFLAGS) $(OFLAGS) pqsort.cpp -o pqsort
	@$(CC) $(CFLAGS) $(OFLAGS) sprefixSum.c -o sprefixSum

clean: prefixSum.o sprefixSum.o
	@rm prefixSum.o sprefixSum.o pqsort.o
