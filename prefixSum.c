#include <stdio.h>
#include <pthread.h>

#include <stdlib.h>
#include <time.h>

#define INPUT_SIZE 100000000

int input[INPUT_SIZE];

int main() {
	srand(time(NULL));
	for(int i = 0; i < INPUT_SIZE; i++) {
		input[i] = rand() % INPUT_SIZE;
	}
	return 0;
}
