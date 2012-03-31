CC = g++
CFLAGS = -g
OFLAGS = -lpthread

all: prefixsum
	  
prefixsum: 
	@$(CC) $(CFLAGS) $(OFLAGS) prefixSum.cpp -o prefixSum
	@$(CC) $(CFLAGS) $(OFLAGS) prefixSumO.cpp -o prefixSumO
	@$(CC) $(CFLAGS) $(OFLAGS) prefixSumS.cpp -o prefixSumS
	@$(CC) $(CFLAGS) $(OFLAGS) pqsort.cpp -o pqsort
	@$(CC) $(CFLAGS) $(OFLAGS) sprefixSum.c -o sprefixSum

clean: 
	@rm prefixSumS.o prefixSumO.o prefixSum.o sprefixSum.o pqsort.o
