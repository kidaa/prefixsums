#include <stdio.h>
#include <pthread.h>
#include <math.h>

#include <stdlib.h>
#include <time.h>

#define INPUT_SIZE 512
#define NUM_THREADS 256

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

logbarrier_t upPhaseBarr, downPhaseBarr, endPhaseBarr;
int input[INPUT_SIZE];
int prefix[INPUT_SIZE];
int check[INPUT_SIZE];
//int prefix[INPUT_SIZE]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
//int input[INPUT_SIZE]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
//int check[INPUT_SIZE]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
//int prefix[INPUT_SIZE]={3,1,7,0,4,1,6,3};
//int input[INPUT_SIZE]={3,1,7,0,4,1,6,3};
//int check[INPUT_SIZE]={3,1,7,0,4,1,6,3};

void init_barrier(logbarrier_t b) {
	for (int i = 0; i < NUM_THREADS; i++) {
		b[i].count = 0;
		pthread_mutex_init(&(b[i].count_lock), NULL);
		pthread_cond_init(&(b[i].ok_to_proceed_up), NULL);
		pthread_cond_init(&(b[i].ok_to_proceed_down), NULL);
	}
}


void logbarrier (logbarrier_t b, int num_threads, int thread_id) {
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
                } else {
                        pthread_mutex_lock(&(b[index].count_lock));
                        b[index].count ++;
                        if (b[index].count == 2)
                           pthread_cond_signal(&(b[index].ok_to_proceed_up));
/*
			while (b[index].count != 0)
*/
			while (
                               pthread_cond_wait(&(b[index].ok_to_proceed_down),
                                    &(b[index].count_lock)) != 0);
			pthread_mutex_unlock(&(b[index].count_lock));
			break;
                }
                base = base + NUM_THREADS/i;
                i = i * 2; 
        } while (i <= NUM_THREADS);
	i = i / 2;
        for (; i > 1; i = i / 2)
        {
		base = base - NUM_THREADS/i;
                index = base + thread_id / i;
                pthread_mutex_lock(&(b[index].count_lock));
                b[index].count = 0;
                pthread_cond_signal(&(b[index].ok_to_proceed_down));
                pthread_mutex_unlock(&(b[index].count_lock));
        }
}


void downPhase(prefixSumMsg msg) {
	int id    = 2 * (msg.thread_id+1);
	int limit = log2(INPUT_SIZE);
	int inc_1, inc, left, right, temp;
	int num_threads = NUM_THREADS;
	for(int depth = limit-1; depth >= 0; depth--) {
		inc_1 = (int)pow(2, depth+1);
		inc   = (int)pow(2, depth);
		if(id % inc_1 != 0) {
			printf("Thread Idle: %d num_threads: %d\n",msg.thread_id, num_threads);
			logbarrier(downPhaseBarr, num_threads, msg.thread_id);
			continue;
		}		
		printf("--Thread %d: Depth %d num_threads:%d\n",msg.thread_id, depth, num_threads);
		left = id-inc-1;
		right = id-1;
		printf("--Thread %d: Depth %d left:%d, right:%d, \n",msg.thread_id, depth, left, right);
		temp = prefix[left];
		prefix[left] = prefix[right];
		prefix[right] += temp;
		logbarrier(downPhaseBarr, num_threads, msg.thread_id);
	}
	logbarrier(endPhaseBarr, NUM_THREADS, msg.thread_id);
	//printf("--Thread %d: Adding left: %d and right: %d\n",msg.thread_id,left, right);
	input[left] += prefix[left];
	input[right] += prefix[right];
}


void upPhase(prefixSumMsg msg) {
	int id    = 2 * msg.thread_id;
	int limit = log2(INPUT_SIZE);
	int inc_1, inc, left, right;
	int num_threads = NUM_THREADS;
	for(int depth = 0; depth < limit; depth++) {
		printf("Thread %d: Depth %d num_threads:%d\n",msg.thread_id, depth, num_threads);
		inc_1 = (int)pow(2, depth+1);
		inc   = (int)pow(2, depth);
		left = id+inc-1;
		right = id+inc_1-1;
		prefix[right] = prefix[left]+prefix[right];
		logbarrier(upPhaseBarr, num_threads, msg.thread_id);
		if((msg.thread_id+1)%inc_1 != 0) {
			printf("Thread Idle: %d num_threads: %d\n",msg.thread_id, num_threads);
			for(depth++; depth < limit; depth++) {
				logbarrier(upPhaseBarr, num_threads, msg.thread_id);
			}
		} else {
			id -= inc_1;
		}
	}
}

void *calcPrefixSum(void* ps_msg) {
	printf("Starting up phase.......\n");
	prefixSumMsg msg = *(prefixSumMsg *)ps_msg;
	upPhase(msg);
	logbarrier(downPhaseBarr, NUM_THREADS, msg.thread_id);
	prefix[INPUT_SIZE-1] = 0;
	printf("last element: %d\n",prefix[INPUT_SIZE-1]);
	logbarrier(downPhaseBarr, NUM_THREADS, msg.thread_id);
	printf("Starting down phase.......\n");
	downPhase(msg);
}


void prefixSum(int* a, const int len, const int num_threads) {
	/* create num_threads threads */
	pthread_t threads[NUM_THREADS];
	prefixSumMsg ps_msg[NUM_THREADS];
	init_barrier(upPhaseBarr);
	init_barrier(downPhaseBarr);
	init_barrier(endPhaseBarr);
	for(int i = 0; i < num_threads; i++) {
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
	for(int i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}
}


int main() {
	int sum[NUM_THREADS];
	srand(time(NULL));
	for(int i = 0; i < INPUT_SIZE; i++) {
		check[i] = prefix[i] = input[i] = rand() % 2;
	}
	time_t start, end;
	time(&start);
	prefixSum(input, INPUT_SIZE, NUM_THREADS);
	time(&end);
	printf("Parallel Time: %0.2f\n", difftime(end, start));
	time(&start);
	for(int i = 1; i < INPUT_SIZE; i++) {
		check[i] += check[i-1];
	}
	time(&end);
	printf("Serial Time: %0.2f\n", difftime(end, start));
	for(int i=0;i<INPUT_SIZE;i++){
		if(input[i] != check[i]){
			printf("------FAIL:%d %d-------\n",prefix[i], check[i]);
		}
		//printf("%d %d\n",input[i], check[i]);
	}
	printf("\n");
	return 0;
}
