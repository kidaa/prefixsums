#include <stdio.h>
#include <pthread.h>
#include <math.h>

#include <stdlib.h>
#include <time.h>

#define INPUT_SIZE 16
#define NUM_THREADS 16

typedef struct {
	int thread_id;
	int level;
} prefixSumMsg;

typedef struct barrier_node {
	pthread_mutex_t count_lock;
	pthread_cond_t ok_to_proceed_up;
	pthread_cond_t ok_to_proceed_down;
	int count;
} barrier_t_internal;

typedef struct barrier_node logbarrier_t[NUM_THREADS];

barrier_t barr;
int input[INPUT_SIZE];
int prefix[INPUT_SIZE]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

void init_barrier(logbarrier_t b) {
	for (int i = 0; i < MAX_THREADS; i++) {
		b[i].count = 0;
		pthread_mutex_init(&(b[i].count_lock), NULL);
		pthread_cond_init(&(b[i].ok_to_proceed_up), NULL);
		pthread_cond_init(&(b[i].ok_to_proceed_down), NULL);
	}
}


void logbarrier(logbarrier_t b, int num_threads, int thread_id) {
	int i, base, index;
	i = 2;
	base = 0;
        do {
		index = base + thread_id / i;
								                if (thread_id % i == 0) {
											                        pthread_mutex_lock(&(b[index].count_lock));
														                        b[index].count ++;
																	                        while (b[index].count < 2)
																					                              pthread_cond_wait(&(b[index].ok_to_proceed_up),
																										                                              &(b[index].count_lock));
																				                        pthread_mutex_unlock(&(b[index].count_lock));
																							                }
										                else {
													                        pthread_mutex_lock(&(b[index].count_lock));


void *calcPrefixSum(void* ps_msg) {
	prefixSumMsg msg = *(prefixSumMsg *)ps_msg;
	int id    = 2 * msg.thread_id;
	int limit = log2(INPUT_SIZE);
	int inc_1, inc, left, right;
	int num_threads = NUM_THREADS/2;
	for(int depth = 0; depth < limit; depth++) {
		printf("Thread %d: Depth %d num_threads:%d\n",msg.thread_id, depth, num_threads);
		inc_1 = (int)pow(2, depth+1);
		inc   = (int)pow(2, depth);
		left = id+inc-1;
		right = id+inc_1-1;
		prefix[right] = prefix[left]+prefix[right];
//		int keepalive = (int) pow(2, depth+1);
		barrier(&barr, num_threads);
		if((msg.thread_id+1)%inc_1 != 0) {
			printf("Thread Exit: %d num_threads: %d\n",msg.thread_id, num_threads);
			pthread_exit(0);
		}
		id -= inc_1;
		num_threads /= 2;
	}
}


void prefixSum(int* a, const int len, const int num_threads) {
	/* create (num_threads/2) threads */
	pthread_t threads[NUM_THREADS/2];
	prefixSumMsg ps_msg[NUM_THREADS/2];
	init_barrier(&barr);
	for(int i = 0; i < (num_threads/2); i++) {
		ps_msg[i].thread_id = i;
		ps_msg[i].level = 0;
		printf("Creating thread %d\n",i);
		int rc = pthread_create(&threads[i], NULL, calcPrefixSum, 
				        (void *) &ps_msg[i]);
		if(rc == -1) {
			printf("prefixSum: pthread_create\n",__func__);
			exit(0);
		}
	}
	for(int i = 0; i < (NUM_THREADS/2); i++) {
		pthread_join(threads[i], NULL);
	}
}

int main() {
	int sum[NUM_THREADS];
	srand(time(NULL));
	for(int i = 0; i < INPUT_SIZE; i++) {
		input[i] = rand() % 2;
	}
	prefixSum(input, INPUT_SIZE, NUM_THREADS);
	for(int i = 1; i < INPUT_SIZE; i++) {
		input[i] += input[i-1];
	}

	for(int i=0;i<INPUT_SIZE;i++){
		if(input[i] != prefix[i]){
		}
		printf("%d ",prefix[i]);
	}
	printf("\n");
	return 0;
}
