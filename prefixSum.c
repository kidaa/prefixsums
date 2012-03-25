#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <sys/time.h>

#define INPUT_SIZE 8
#define NUM_THREADS 8
#define DEBUG

typedef struct {
	int thread_id;
	int start;
	int end;
} prefixSumMsg;

struct barrier_node {
	pthread_mutex_t count_lock;
	pthread_cond_t ok_to_proceed_up;
	pthread_cond_t ok_to_proceed_down;
	int count;
};

typedef struct barrier_node logbarrier_t[NUM_THREADS];

logbarrier_t upPhaseBarr, downPhaseBarr;
//int input [INPUT_SIZE];
//int check [INPUT_SIZE];
int   sum [2*NUM_THREADS];
//int input[INPUT_SIZE]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
//int check[INPUT_SIZE]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
int input[INPUT_SIZE]={3,1,7,0,4,1,6,3};
int check[INPUT_SIZE]={3,1,7,0,4,1,6,3};

void init_barrier(logbarrier_t b) {
	for (int i = 0; i < NUM_THREADS; i++) {
		b[i].count = 0;
		pthread_mutex_init(&(b[i].count_lock), NULL);
		pthread_cond_init(&(b[i].ok_to_proceed_up), NULL);
		pthread_cond_init(&(b[i].ok_to_proceed_down), NULL);
	}
}


void logbarrier (logbarrier_t b,  const int thread_id) {
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


void downPhase(const prefixSumMsg msg) {
	int id    = 2 * (msg.thread_id+1);
	int limit = log2(2*NUM_THREADS);
	int inc_1 = (1 << limit), i, inc, left, right, temp, depth;
	for(depth = limit-1; depth >= 0; depth--) {
		inc   = inc_1/2;
		if(id % inc_1 == 0) {
			left = id-inc-1;
			right = id-1;
#ifndef DEBUG
			printf("--Thread %d: Depth %d left:%d, right:%d, \n",msg.thread_id, depth, left, right);
#endif
			temp = sum[left];
			sum[left] = sum[right];
			sum[right] += temp;
		}
		logbarrier(downPhaseBarr, msg.thread_id);
		inc_1 = inc;
	}
	logbarrier(upPhaseBarr, msg.thread_id);
#ifndef DEBUG
	printf("xxxxThread : %d start: %d end: %d\n",msg.thread_id , msg.start, msg.end);
#endif
	temp = (msg.end - msg.start)/2;
	if(temp == 0) {
#ifndef DEBUG
		printf("--Thread %d: left:%d, right:%d, input[left]:%d input[right]:%d\n",msg.thread_id, left, right, input[left], input[right]);
#endif
		input[msg.start]   += sum[left];
		if(msg.start != msg.end) 
		input[msg.end]     += sum[right];
	} else {
		depth = msg.start + temp;
		for(i = msg.start; i < depth; i++) {
#ifndef DEBUG
		printf("--Thread %d: Adding i: %d left:%d, input[i]:%d sum[left]:%d\n",msg.thread_id, i, left, input[i], sum[left]);
#endif
			input[i] += sum[left];
		}
		for(i = depth; i <= msg.end; i++) {
#ifndef DEBUG
	printf("--Thread %d: Adding i: %d right:%d, input[i]:%d sum[right]:%d\n",msg.thread_id, i, right, input[i], sum[right]);
#endif
			input[i] += sum[right];
		}
	}
	
}


void upPhase(const prefixSumMsg msg) {
	int id    = 2 * msg.thread_id;
	int limit = log2(2*NUM_THREADS);
	int inc_1 = 2, inc, left, right, depth;
	for(depth = 0; depth < limit; depth++) {
		//printf("Thread %d: Depth %d num_threads:%d\n",msg.thread_id, depth, num_threads);
		inc   = inc_1/2;
		if((msg.thread_id+1)%inc == 0) {
			left = id+inc-1;
			right = id+inc_1-1;
			sum[right] = sum[left]+sum[right];
			if((depth == limit-1) && (right == 2*NUM_THREADS-1)) {
				sum[right] = 0;
			}
			id -= inc_1;
			inc_1 *= 2;
		}
		logbarrier(upPhaseBarr, msg.thread_id);
	}
}


void *calcPrefixSum(void* ps_msg) {
	prefixSumMsg msg = *(prefixSumMsg *)ps_msg;
	int s = 0, i;
	int half = (msg.end - msg.start)/2;
	if(half == 0) {
		if(msg.start == msg.end) {
			sum[2*msg.thread_id]   = 0;	
			sum[2*msg.thread_id+1] = input[msg.start];
		} else {
			sum[2*msg.thread_id]   = input[msg.start];	
			sum[2*msg.thread_id+1] = input[msg.end];
		}
	} else {
		int l = msg.start+half;
		s += input[msg.start];
		for(i = msg.start+1; i < l; i++) {
			s += input[i];
			input[i] += input[i-1];
		}
		sum[2*msg.thread_id] = s;
		for(s = input[l],i = l+1; i <= msg.end; i++) {
			s += input[i];
			input[i] += input[i-1];
		}
		sum[2*msg.thread_id+1] = s;
	}

#ifndef DEBUG
	printf("//////Thread : %d Sum: %d %d\n",msg.thread_id ,sum[2*msg.thread_id], sum[2*msg.thread_id+1]);
#endif
	logbarrier(upPhaseBarr,  msg.thread_id);
#ifndef DEBUG
//	printf("Starting up phase.......\n");
#endif
	upPhase(msg);
	logbarrier(downPhaseBarr, msg.thread_id);

//	sum[2*NUM_THREADS-1] = 0;
//	logbarrier(downPhaseBarr, NUM_THREADS, msg.thread_id);
//
#ifndef DEBUG
	printf("Starting down phase.......\n");
#endif

	downPhase(msg);
}


void prefixSum(int* a, const int len, const int num_threads) {
	/* create num_threads threads */
	pthread_t threads[NUM_THREADS];
	prefixSumMsg ps_msg[NUM_THREADS];
	init_barrier(upPhaseBarr);
	init_barrier(downPhaseBarr);
	int group = len / num_threads;
	int rem   = len % num_threads;
	int start = 0, end = -1;
	for(int i = 0; i < num_threads; i++) {
		ps_msg[i].thread_id = i;
		start = end + 1;
		end = start + group - 1;
		if(rem > 0) {
			rem--;
			end++;
		}
		ps_msg[i].start = start;
		ps_msg[i].end   = end;
		//printf("Creating thread %d with start %d end %d\n",i, start, end);
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
	srand(time(NULL));
	for(int i = 0; i < INPUT_SIZE; i++) {
//		 check[i] = input[i] = rand()%2;
	}
	clock_t start, end;
	double stime, ptime;
        struct timeval tz;
        struct timezone tx;
        double start_time, end_time;
        gettimeofday(&tz, &tx);
        start_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;

	start = clock();
	prefixSum(input, INPUT_SIZE, NUM_THREADS);
	end = clock();
	ptime = end-start;

        gettimeofday(&tz, &tx);
        end_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;
	double pTime = end_time-start_time;
	printf("Parallel Time: cycles - %lf time_p - %lf\n", ptime, end_time - start_time);

#ifndef DEBUG
	for(int i=0;i<2*NUM_THREADS;i++){
		printf("%d ",sum[i]);
	}
	printf("\n");
#endif

        gettimeofday(&tz, &tx);
        start_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;

	start = clock();
	for(int i = 1; i < INPUT_SIZE; i++) {
		check[i] += check[i-1];
	}
	end = clock();

        gettimeofday(&tz, &tx);
        end_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;
	stime = end-start;
	double sTime = end_time-start_time;
	printf("Serial Time: cycles - %lf time_s - %lf\n", stime, end_time - start_time);

	for(int i=0;i<INPUT_SIZE;i++){
		if(input[i] != check[i]){
			printf("------FAIL:%d %d-------\n",input[i], check[i]);
		}
//		printf("%d %d\n",input[i], check[i]);
	}
	
	printf("Speedup: %lf\n",(double)sTime/pTime);
	printf("\n");
	
	return 0;
}
