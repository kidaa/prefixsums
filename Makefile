CC = g++
CFLAGS = -g
OFLAGS = -lpthread

all: prefixsum
	  
prefixsum: 
	@$(CC) $(CFLAGS) $(OFLAGS) prefixSum.cpp -o prefixSum
	@$(CC) $(CFLAGS) $(OFLAGS) pqsort.cpp -o pqsort
	@$(CC) $(CFLAGS) $(OFLAGS) sprefixSum.c -o sprefixSum

clean: 
	@rm prefixSum.o sprefixSum.o pqsort.o
