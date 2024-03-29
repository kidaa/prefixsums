#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define INPUT_SIZE 100000000

int input[INPUT_SIZE];


int main() {
	 struct timeval tz;
	 struct timezone tx;
	 double start_time, end_time;

	for(int i = 0; i < INPUT_SIZE; i++) {
		input[i] = rand() % 10;
	}
	
	gettimeofday(&tz, &tx);
	start_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;
	for(int i = 1; i < INPUT_SIZE; i++) {
		input[i] += input[i-1];
	}
	gettimeofday(&tz, &tx);
	end_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;
					  
	printf("Serial Time: %lf\n", end_time - start_time);
	return 0;
}
