all: 
	@gcc  -o pqsort -lpthread -lm pqsort.c
clean:
	@rm pqsort

