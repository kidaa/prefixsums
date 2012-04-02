#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include <iostream>
using namespace std;

#define INPUT_SIZE 10000000
#define NUM_THREADS 4
#define median(a,n) kth_smallest(a,n,(((n)&1)?((n)/2):(((n)/2)-1)))
#define printf(x,...) 
#define DEBUG

typedef struct {
	int thread_id;
	int pstart;
	int start;
	int end;
	int pend;
	int first_thread;
	int last_thread;
} prefixSumMsg;

pthread_barrier_t cbarr[NUM_THREADS], obarr[NUM_THREADS];
pthread_t threads[NUM_THREADS];

int sum1 [NUM_THREADS];
int sum2 [NUM_THREADS];
int medians [NUM_THREADS];
int pivots [NUM_THREADS];
int pivots_indices [NUM_THREADS];
//int input1[INPUT_SIZE]={2, 3, 5, 5, 7, 6, 1, 7};
//int check[INPUT_SIZE]={1, 2, 3, 1};
//int check[INPUT_SIZE]={2, 3, 5, 5, 7, 6, 1, 7};
int input2 [INPUT_SIZE];
int input1 [INPUT_SIZE];
int input3 [INPUT_SIZE];
int check [INPUT_SIZE];


prefixSumMsg ps_msg[NUM_THREADS];

//int input1[INPUT_SIZE]={13,12,17,15,14,11,16,18};
//int check[INPUT_SIZE]={3,1,7,0,4,1,6,3};

int kth_smallest(int a[], int n, int k)
{
	int i,j,l,m ;
	int x, t ;

    l=0 ; m=n-1 ;
    while (l<m) {
        x=a[k] ;
        i=l ;
        j=m ;
        do {
            while (a[i]<x) i++ ;
            while (x<a[j]) j-- ;
            if (i<=j) {
            	t = a[i];
            	a[i] = a[j];
            	a[j] = t;
                i++ ; j-- ;
            }
        } while (i<=j) ;
        if (j<k) l=i ;
        if (k<i) m=j ;
    }
    return a[k] ;
}

int comp (const void * a, const void * b) {
  return ( *(int*)a - *(int*)b );
}

void *calcPrefixSum(void* tid) {
	int inc_1, inc, left, temp, i, j, right, depth, cnt1, cnt2;
	int  prev_lt, prev_end, pstart;
	int first, last, num_threads, group1, group2, s, e, rem1, rem2, barr_id, t;
	int k1, k2, new_pend, med;
	int thread_id = *(int *)tid;
	int *from, *to, *tp;
	from = input1;
	to   = input2;

	while(true) {

	printf("\nThread id: %d pstart: %d pend: %d start: %d end: %d first_thread:%d last_thread: %d\n", ps_msg[thread_id].thread_id, ps_msg[thread_id].pstart, ps_msg[thread_id].pend, ps_msg[thread_id].start, ps_msg[thread_id].end, ps_msg[thread_id].first_thread, ps_msg[thread_id].last_thread);

	barr_id = ps_msg[thread_id].first_thread;
	
         if(barr_id == ps_msg[thread_id].last_thread) {
		printf("\n\nThread %d: RETURNED\n\n", thread_id);
		qsort(from+ps_msg[thread_id].pstart, ps_msg[thread_id].pend-ps_msg[thread_id].pstart+1, sizeof(int), comp);
		if(from != input1)
		memcpy((void*)(to+ps_msg[thread_id].pstart), (void*)(from+ps_msg[thread_id].pstart), (ps_msg[thread_id].pend-ps_msg[thread_id].pstart+1)*sizeof(int));
		return NULL;
	}
	
	med = median(from+ps_msg[thread_id].start, ps_msg[thread_id].end-ps_msg[thread_id].start+1);
	medians[thread_id] = med;
	printf("Thread %d: %d\n", thread_id, medians[thread_id]);
	pthread_barrier_wait(&cbarr[barr_id]);	
	
	if(thread_id == barr_id) {
		med = median(medians+barr_id, ps_msg[thread_id].last_thread-barr_id+1);
		pivots[barr_id] = med;
		printf("Global pivot: %d\n", pivots[barr_id]);
	}
	
	pthread_barrier_wait(&cbarr[barr_id]);
	
	/* local rearrangement in each chunk with in the partition 
	 * Dutch National Flag - 2 partitioning */
	i = ps_msg[thread_id].start;
	printf("Start: Thread %d: barr_id: %d loop from %d to %d\n", thread_id, barr_id,ps_msg[thread_id].start,ps_msg[thread_id].end);
	
	for(cnt1 = 0, j = ps_msg[thread_id].end; i <= j;) {
		if(from[i] >= pivots[barr_id]) {
			if(from[j] < pivots[barr_id]) {
				temp = from[i];
				from[i] = from[j];
				from[j] = temp;
			}
			j--;
		} else {
			i++;
			cnt1++;
		}
	}

	sum1[thread_id] = cnt1;
	cnt2 = (ps_msg[thread_id].end - ps_msg[thread_id].start + 1) - cnt1;
	sum2[thread_id] = cnt2;
	
	printf("\nLocal Rearragement on thread: %d cnt1: %d cnt2: %d\n",thread_id, cnt1, cnt2);
#ifndef DEBUG
	for(i = ps_msg[thread_id].start; i <= ps_msg[thread_id].end; i++){
		printf("%d ",from[i]);
	}
#endif
	printf("\n");
	printf(" Thread %d: waiting on %d \n", thread_id, barr_id);
	pthread_barrier_wait(&cbarr[barr_id]);
	
	if(thread_id == barr_id) {
		printf("\nTotal Local Rearragement on %d cnt1: %d cnt2: %d: \n",thread_id, cnt1, cnt2);
#ifndef DEBUG
		for(i = ps_msg[thread_id].pstart; i <= ps_msg[thread_id].pend; i++){
			printf("%d ",from[i]);
		}
		printf("\n");
		printf("Before Prefix Sum1: ");
		for(i = barr_id; i <= ps_msg[thread_id].last_thread; i++){
			printf("%d ",sum1[i]);
		}
		printf("\n");
#endif
		int psum = 0, s = 0;
		for(i = barr_id; i <= ps_msg[thread_id].last_thread; i++) {
			s += sum1[i];
			sum1[i] = psum;
			psum = s;
		}
		pivots_indices[barr_id] = ps_msg[barr_id].pstart + s;
		printf("New pivot index is %d\n",pivots_indices[barr_id]);
#ifndef DEBUG
		printf("Prefix Sum1: ");
		for(i = barr_id; i <= ps_msg[thread_id].last_thread; i++){
			printf("%d ",sum1[i]);
		}
		printf("\n");
#endif

	} else if(thread_id == ps_msg[thread_id].last_thread) {
#ifndef DEBUG
		printf("Before Prefix Sum2: ");
		for(i = barr_id; i <= ps_msg[thread_id].last_thread; i++){
			printf("%d ",sum2[i]);
		}
		printf("\n");
#endif
		int psum = 0, s = 0;
		for(i = barr_id; i <= ps_msg[thread_id].last_thread; i++) {
			s += sum2[i];
			sum2[i] = psum;
			psum = s;
		}
		printf("Prefix Sum2: ");
#ifndef DEBUG
		for(i = barr_id; i <= ps_msg[thread_id].last_thread; i++){
			printf("%d ",sum2[i]);
		}
		printf("\n");	
#endif	
	}
	pthread_barrier_wait(&cbarr[barr_id]);
	/* Copy elements to a new array */

	i = ps_msg[thread_id].start;
	pstart = ps_msg[thread_id].pstart;
	if(cnt1 != 0) { 
		printf("c1-Thread: %d copying to pstart:%d+%d from %d elements-%d to %d\n",thread_id, pstart, sum1[thread_id], i,cnt1, pstart+sum1[thread_id]);
		memcpy((void*)(to+pstart+sum1[thread_id]), (void*)(from+i), cnt1*sizeof(int));
	}
	if(cnt2 != 0) {
		printf("c2-Thread: %d copying to pstart:%d+%d from %d elements-%d to %d\n",thread_id, pstart+pivots_indices[barr_id], sum2[thread_id], i+cnt1, cnt2, pivots_indices[barr_id]+sum2[thread_id]);
		memcpy((void*)(to+pivots_indices[barr_id]+sum2[thread_id]),(void*)(from+i+cnt1), cnt2*sizeof(int));
	}
	pthread_barrier_wait(&cbarr[barr_id]);

	tp = from;
	from = to;
	to   = tp;

	if(thread_id == barr_id) {
		
		prev_end = ps_msg[thread_id].pend;
		prev_lt  = ps_msg[thread_id].last_thread;
#ifndef DEBUG
		printf("Final: ");
		for(i = ps_msg[thread_id].pstart; i <= ps_msg[thread_id].pend; i++){
			printf("%d ",from[i]);
		}
		printf("\n");
#endif
		num_threads = ps_msg[thread_id].last_thread - barr_id + 1;
		k1     = pivots_indices[barr_id] - ps_msg[thread_id].start; 
		k2     = prev_end - pivots_indices[barr_id] + 1; 
		first  = ((double)k1/(k1+k2))*num_threads;
		first  = (first == 0)?1:first;
		last   = num_threads - first;
	
		group1 = k1/first;
		rem1   = k1%first;

		group2 = k2/last;
		rem2   = k2%last;
		
		printf("\n---------Thread: %d First Paritioning: k:%d group:%d rem:%d first: %d last: %d------------\n", thread_id, k1, group1, rem1, first, last);
		printf("loop from: %d to %d\n", barr_id, barr_id+first);
		
		for(i = barr_id, s = 0, e = ps_msg[thread_id].start-1; i < (barr_id+first); i++) {
			s = e + 1;
			e = s + group1 - 1;
			if(rem1 > 0) {
				rem1--;
				e++;
			}
			ps_msg[i].pstart = ps_msg[thread_id].pstart;
			ps_msg[i].pend   = pivots_indices[barr_id]-1;
			ps_msg[i].start  = s;
			ps_msg[i].end    = e;
			ps_msg[i].first_thread = ps_msg[thread_id].first_thread;
			ps_msg[i].last_thread  = ps_msg[thread_id].first_thread+first-1;
		}

		printf("\n---------Thread: %d Second Paritioning: k:%d group:%d rem:%d first: %d last: %d------------\n", thread_id, k2, group2, rem2, first, last);
	printf("loop from: %d to %d\n",  barr_id+first,barr_id+first+last);
		for(i = (barr_id+first), e = pivots_indices[barr_id]-1; i < (barr_id+first+last); i++) {
			s = e + 1;
			e = s + group2 - 1;
			if(rem2 > 0) {
				rem2--;
				e++;
			}
			ps_msg[i].pstart = pivots_indices[barr_id];
			ps_msg[i].pend   = prev_end;
			ps_msg[i].start = s;
			ps_msg[i].end   = e;
			ps_msg[i].first_thread = ps_msg[thread_id].first_thread+first;
			ps_msg[i].last_thread  = ps_msg[thread_id].first_thread+first+last-1;
		}

#ifndef DEBUG
		for(i = 0; i < NUM_THREADS; i++) {
			printf("Thread id: %d pstart: %d pend: %d start: %d end: %d first_thread:%d last_thread: %d\n", ps_msg[i].thread_id, ps_msg[i].pstart, ps_msg[i].pend, ps_msg[i].start, ps_msg[i].end, ps_msg[i].first_thread, ps_msg[i].last_thread);
		}
#endif
		/*
		printf("Finding median from %d to %d at %d\n", ps_msg[thread_id].pstart ,pivots_indices[barr_id], ps_msg[barr_id].pstart+((pivots_indices[barr_id]-1-ps_msg[barr_id].pstart)/2));
#endif
		s = kth_smallest(from, ps_msg[barr_id].pstart, pivots_indices[barr_id],  ps_msg[barr_id].pstart+((pivots_indices[barr_id]-1-ps_msg[barr_id].pstart)/2));
		printf("\n");
		
		pivots[barr_id] = t = from[s];
		from[s] = from[ps_msg[thread_id].pstart];
		from[ps_msg[thread_id].pstart] = t;
		printf("Adding pivot in %d, value: %d\n",barr_id, t);
		printf("Swapping %d and %d\n", s , ps_msg[thread_id].pstart);

		printf("First Median: %d pos: %d Index: %d\n",pivots[barr_id], s, barr_id);
	
		printf("Finding median from %d to %d at %d\n", pivots_indices[barr_id] , prev_end+1, pivots_indices[barr_id]+((prev_end-pivots_indices[barr_id])/2));
		e = kth_smallest(from, pivots_indices[barr_id], prev_end+1, pivots_indices[barr_id]+((prev_end-pivots_indices[barr_id])/2));
		//printf("Median: %d pos: %d\n",from[e], e);

#ifndef DEBUG
		printf("After median: ");
		for(i = ps_msg[i].pstart; i <= prev_end; i++){
			printf("%d ",from[i]);
		}
		printf("\n");
#endif

		pivots[barr_id+first] = t = from[e]; 
		from[e] = from[pivots_indices[barr_id]];
		from[pivots_indices[barr_id]] = t;
		//printf("Adding pivot in %d value: %d\n",barr_id+first, t);
#ifndef DEBUG
		printf("Swapping %d and %d\n", e, pivots_indices[barr_id]);
		printf("Second Median: %d pos: %d Index: %d\n",pivots[barr_id+first], e, barr_id+first);
		printf("Input: ");
		for(i = 0; i < INPUT_SIZE; i++){
			printf("%d ",from[i]);
		}
		
		printf("\n--------------------------------------------------------\n\n");
#endif
*/
		printf("cbarr: %d %d\n",barr_id, first);
		pthread_barrier_destroy(&cbarr[barr_id]);
		pthread_barrier_init(&cbarr[barr_id], NULL, first);
		pthread_barrier_destroy(&cbarr[barr_id+first]);
		printf("cbarr: %d %d\n",barr_id+first, last);
		pthread_barrier_init(&cbarr[barr_id+first], NULL, last);
		for(i = barr_id+1; i <= prev_lt; i++) {
			pthread_barrier_wait(&obarr[i]);
		}
		printf("Thread id: %d Master thread done !!\n", thread_id);
	} else {
		pthread_barrier_wait(&obarr[thread_id]);
		printf("Thread %d done with a level\n",thread_id);
	  }

	}
}



void spawn_threads(const int len, const int num_threads) {
	int i, t;
	for(i = 0; i < NUM_THREADS; i++) {
		pthread_barrier_init(&cbarr[i], NULL, NUM_THREADS);
		pthread_barrier_init(&obarr[i], NULL, 2);
	}
	int group = len / num_threads;
	int rem   = len % num_threads;
	int s = 0, e = -1;
	
	
	printf("Input: ");
	for(int i = 0; i < INPUT_SIZE; i++){
			printf("%d ",input1[i]);
	}
	printf("\n");
/*	
	i = kth_smallest(input1, 0, INPUT_SIZE, INPUT_SIZE/2);
	pivots[0] = input1[i];	
	
	printf("Median: %d pos: %d\n",input1[i], i);
	printf("After median ");
	for(int i = 0; i < INPUT_SIZE; i++){
			printf("%d ",input1[i]);
	}
	printf("\n");
	t = input1[0];
	input1[0] = input1[i];
	input1[i] = t;
	for(int i = 0; i < INPUT_SIZE; i++){
			printf("%d ",input1[i]);
	}
	printf("\n");
*/
	for(i = 0; i < num_threads; i++) {
		s = e + 1;
		e = s + group - 1;
		if(rem > 0) {
			rem--;
			e++;
		}
		ps_msg[i].thread_id  = i;
		ps_msg[i].start = s;
		ps_msg[i].end   = e;
		ps_msg[i].pstart = 0;
		ps_msg[i].pend   = len-1;
		ps_msg[i].last_thread = NUM_THREADS-1;
		ps_msg[i].first_thread = 0;
		int rc = pthread_create(&threads[i], NULL, calcPrefixSum, 
				        (void *)& ps_msg[i].thread_id);
		if(rc == -1) {
			printf("prefixSum: pthread_create\n",__func__);
			exit(EXIT_FAILURE);
		}
	}
	for(i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}
}


int main() {
    srand(time(NULL));
    for(int i = 0; i < INPUT_SIZE; i++) {
	input3[i] = check[i] = input1[i] = rand()%INPUT_SIZE;
    }
    double sTime, pTime;
    struct timeval tz;
    struct timezone tx;
    double start_time, end_time;
    gettimeofday(&tz, &tx);
    start_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;

    spawn_threads(INPUT_SIZE, NUM_THREADS);

    gettimeofday(&tz, &tx);
    end_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;
    pTime = end_time-start_time;
    cout<<"Parallel Time: time_p "<< pTime<<"\n";
    
    gettimeofday(&tz, &tx);
    start_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;
	qsort(check, INPUT_SIZE, sizeof(int),comp); 
     gettimeofday(&tz, &tx);
    end_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;
    sTime = end_time-start_time;
   cout<<"Serial Time: time_s "<< sTime<<"\n";
    
    for(int i=0;i<INPUT_SIZE;i++){
			//printf("%d %d\n", input1[i], check[i]);
			if(input1[i] != check[i]) {
			  cout<<"FAIL?????????????????????????????\nInput:\n";
			  for(int k = 0; k < INPUT_SIZE; k++)
				cout<<input3[k]<<" ";
			 cout<<"\n";
			
			  cout<<"Ouput1:\n";
			  for(int k = 0; k < INPUT_SIZE; k++)
					cout<<input1[k]<<", ";
			cout<<"\n";
			 cout<<"Ouput2:\n";
			  for(int k = 0; k < INPUT_SIZE; k++)
					cout<<input2[k]<<", ";
			cout<<"\n"; /*
			cout<<"Expected answer:\n";
			for(int k = 0; k < INPUT_SIZE; k++)
					cout<<check[k]<<", ";*/
			  break;
			}
    }
    printf("\n");
    cout<<"Speedup: "<<(double)sTime/pTime<<"\n";
	
	/*cout<<"Ouput:\n";
			  for(int k = 0; k < INPUT_SIZE; k++)
					cout<<input2[k]<<", ";
			cout<<"\n";
			cout<<"Expected answer:\n";
			for(int k = 0; k < INPUT_SIZE; k++)
					cout<<check[k]<<", ";
							cout<<"\n";
							*/
    return EXIT_SUCCESS;
}
