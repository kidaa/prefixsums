#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INPUT_SIZE 100000000

int input[INPUT_SIZE];

void serialPrefixSum() {
	for(int i = 1; i < INPUT_SIZE; i++) {
		input[i] += input[i-1];
	}
}

int main() {
	srand(time(NULL));
	for(int i = 0; i < INPUT_SIZE; i++) {
		input[i] = rand() % INPUT_SIZE;
	}
	serialPrefixSum();
	return 0;
}
