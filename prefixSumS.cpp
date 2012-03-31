#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <sys/time.h>

#define INPUT_SIZE 8
#define NUM_THREADS 8 

typedef struct {
	int thread_id;
	int start;
	int end;
	int pstart;
	int pend;
	int num_threads;
	int first_thread;
} prefixSumMsg;

pthread_barrier_t barr;
int input [INPUT_SIZE];
int check [INPUT_SIZE];
int sum[NUM_THREADS];
int newLimit;
//int input[INPUT_SIZE]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
//int check[INPUT_SIZE]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
//int input[INPUT_SIZE]={3,1,7,0,4,1,6,3};
//int check[INPUT_SIZE]={3,1,7,0,4,1,6,3};



void *calcPrefixSum(void* ps_msg) {
	int inc_1, inc, left, temp, i, right, depth, limit = newLimit;
	prefixSumMsg msg = *(prefixSumMsg *)ps_msg;
	for(int i = msg.start+1; i <= msg.end; i++) {
		input[i] += input[i-1];
	}
	sum[msg.thread_id] = (msg.start > msg.end)? 0: input[msg.end];
	
	pthread_barrier_wait(&barr);
	
	if(msg.thread_id == 0) {
		int psum = 0, s = 0;
		for(i = 0; i < NUM_THREADS; i++) {
			s += sum[i];
			sum[i] = psum;
			psum = s;
		}
	}
	pthread_barrier_wait(&barr);

	for(i = msg.start; i <= msg.end; i++) {
		input[i] += sum[msg.thread_id];
	}
}


void spawn_threads(int* a, const int len, const int num_threads) {
	pthread_t threads[NUM_THREADS];
	prefixSumMsg ps_msg[NUM_THREADS];
	pthread_barrier_init(&barr, NULL, NUM_THREADS);
	int group = len / num_threads;
	int rem   = len % num_threads;
	int start = 0, end = -1;
	for(int i = 0; i < num_threads; i++) {
		start = end + 1;
		end = start + group - 1;
		if(rem > 0) {
			rem--;
			end++;
		}
		ps_msg[i].thread_id = i;
		ps_msg[i].start  = start;
		ps_msg[i].end    = end;
		ps_msg[i].pstart = 0;
		ps_msg[i].pend   = len-1;
		ps_msg[i].num_threads = num_threads;
		ps_msg[i].first_thread = 0;
		int rc = pthread_create(&threads[i], NULL, calcPrefixSum, 
				        (void *) &ps_msg[i]);
		if(rc == -1) {
			printf("prefixSum: pthread_create\n",__func__);
			exit(EXIT_FAILURE);
		}
	}
	for(int i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}
}


int main() {
    srand(time(NULL));
    for(int i = 0; i < INPUT_SIZE; i++) {
//	 check[i] = input[i] = rand()%2;
    }
    newLimit = ceil(log2(NUM_THREADS));
    double sTime, pTime;
    struct timeval tz;
    struct timezone tx;
    double start_time, end_time;
    gettimeofday(&tz, &tx);
    start_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;

    spawn_threads(input, INPUT_SIZE, NUM_THREADS);

    gettimeofday(&tz, &tx);
    end_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;
	pTime = end_time-start_time;
	printf("Parallel Time: time_p - %lf\n", pTime);
/*
	for(int i = 0; i < sumLimit; i++){
		printf("%d ",sum[i]);
	}
	printf("\n");
*/
    gettimeofday(&tz, &tx);
    start_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;

	for(int i = 1; i < INPUT_SIZE; i++) {
		check[i] += check[i-1];
	}

    gettimeofday(&tz, &tx);
    end_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;
	sTime = end_time-start_time;
	printf("Serial Time: time_s - %lf\n", sTime);

	for(int i=0;i<INPUT_SIZE;i++){
		if(input[i] != check[i]){
			printf("------FAIL:%d %d-------\n",input[i], check[i]);
		}
	//	printf("%d %d\n",input[i], check[i]);
	}
	printf("Speedup: %lf\n",(double)sTime/pTime);
	
	return EXIT_SUCCESS;
}
