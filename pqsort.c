#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

#define INPUT_SIZE 10000000
#define median(a,n) kth_smallest(a,n,(((n)&1)?((n)/2):(((n)/2)-1)))
#define ELEM_SWAP(a,b) { t=(a);(a)=(b);(b)=t; }
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

pthread_barrier_t *cbarr, *obarr;
pthread_t *threads;
prefixSumMsg *ps_msg;

int num_threads;
int *sum1, *sum2;
int *medians, *pivots, *pivots_indices;
int *input1, *input2;

int kth_smallest(int a[], int n, int k) {
	register i,j,l,m ;
	register int x, t;

    l=0 ; m=n-1 ;
    while (l<m) {
        x=a[k] ;
        i=l ;
        j=m ;
        do {
            while (a[i]<x) i++ ;
            while (x<a[j]) j-- ;
            if (i<=j) {
            	ELEM_SWAP(a[i],a[j]) ;
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
	int left, i, j, right, depth = 0, cnt1, cnt2;
	int  prev_lt, prev_end, pstart;
	int first, last,  group, s, e, rem, barr_id;
	int k1, k2,  med;
	int thread_id = *(int *)tid;
	 int *from = input1, *to = input2, *tp;
	register int t;

	while(1) {
#ifndef DEBUG
	printf("\nThread id: %d pstart: %d pend: %d start: %d end: %d first_thread:%d last_thread: %d\n", ps_msg[thread_id].thread_id, ps_msg[thread_id].pstart, ps_msg[thread_id].pend, ps_msg[thread_id].start, ps_msg[thread_id].end, ps_msg[thread_id].first_thread, ps_msg[thread_id].last_thread);
#endif
	barr_id = ps_msg[thread_id].first_thread;
	
         if(barr_id == ps_msg[thread_id].last_thread) {
#ifndef DEBUG
		printf("\n\nThread %d: RETURNED\n\n", thread_id);
#endif
		qsort(from+ps_msg[thread_id].pstart, ps_msg[thread_id].pend-ps_msg[thread_id].pstart+1, sizeof(int), comp);
		if(depth % 2 != 0)
			memcpy((void*)(to+ps_msg[thread_id].pstart), (void*)(from+ps_msg[thread_id].pstart), (ps_msg[thread_id].pend-ps_msg[thread_id].pstart+1)*sizeof(int));
		return NULL;
	}
	
	med = median(from+ps_msg[thread_id].start, ps_msg[thread_id].end-ps_msg[thread_id].start+1);
	medians[thread_id] = med;
#ifndef DEBUG
	printf("Thread %d: %d\n", thread_id, medians[thread_id]);
#endif
	pthread_barrier_wait(&cbarr[barr_id]);	
	
	if(thread_id == barr_id) {
		med = median(medians+barr_id, ps_msg[thread_id].last_thread-barr_id+1);
		pivots[barr_id] = med;
#ifndef DEBUG
		printf("Global pivot: %d\n", pivots[barr_id]);
#endif
	}
	
	pthread_barrier_wait(&cbarr[barr_id]);
	
	/* local rearrangement in each chunk with in the partition 
	 * Dutch National Flag - 2 partitioning */
	i = ps_msg[thread_id].start;
#ifndef DEBUG
	printf("Start: Thread %d: barr_id: %d loop from %d to %d\n", thread_id, barr_id,ps_msg[thread_id].start,ps_msg[thread_id].end);
#endif	
	for(cnt1 = 0, j = ps_msg[thread_id].end; i <= j;) {
		if(from[i] >= pivots[barr_id]) {
			if(from[j] < pivots[barr_id])
				ELEM_SWAP(from[i], from[j]);
			j--;
		} else {
			i++;
			cnt1++;
		}
	}

	sum1[thread_id] = cnt1;
	cnt2 = (ps_msg[thread_id].end - ps_msg[thread_id].start + 1) - cnt1;
	sum2[thread_id] = cnt2;

#ifndef DEBUG	
	printf("\nLocal Rearragement on thread: %d cnt1: %d cnt2: %d\n",thread_id, cnt1, cnt2);
	for(i = ps_msg[thread_id].start; i <= ps_msg[thread_id].end; i++){
		printf("%d ",from[i]);
	}
	printf("\n");
	printf(" Thread %d: waiting on %d \n", thread_id, barr_id);
#endif

	pthread_barrier_wait(&cbarr[barr_id]);
	
	if(thread_id == barr_id) {
#ifndef DEBUG
		printf("\nTotal Local Rearragement on %d cnt1: %d cnt2: %d: \n",thread_id, cnt1, cnt2);
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
		for(k1 = 0, k2 = 0, i = barr_id; i <= ps_msg[thread_id].last_thread; i++) {
			k2 += sum1[i];
			sum1[i] = k1;
			k1 = k2;
		}
		pivots_indices[barr_id] = ps_msg[barr_id].pstart + k2;
#ifndef DEBUG
		printf("New pivot index is %d\n",pivots_indices[barr_id]);
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
		for(k1 = 0, k2 = 0,i = barr_id; i <= ps_msg[thread_id].last_thread; i++) {
			k2 += sum2[i];
			sum2[i] = k1;
			k1 = k2;
		}
#ifndef DEBUG
		printf("Prefix Sum2: ");
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
#ifndef DEBUG
		printf("c1-Thread: %d copying to pstart:%d+%d from %d elements-%d to %d\n",thread_id, pstart, sum1[thread_id], i,cnt1, pstart+sum1[thread_id]);
#endif
		memcpy((void*)(to+pstart+sum1[thread_id]), (void*)(from+i), cnt1*sizeof(int));
	}
	if(cnt2 != 0) {
#ifndef DEBUG
		printf("c2-Thread: %d copying to pstart:%d+%d from %d elements-%d to %d\n",thread_id, pstart+pivots_indices[barr_id], sum2[thread_id], i+cnt1, cnt2, pivots_indices[barr_id]+sum2[thread_id]);
#endif
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
		med = ps_msg[thread_id].last_thread - barr_id + 1;
		k1     = pivots_indices[barr_id] - ps_msg[thread_id].start; 
		k2     = prev_end - pivots_indices[barr_id] + 1; 
		first  = round(((double)k1/(k1+k2))*med);
		first  = (first == 0)?1:first;
		last   = med - first;
	
		group = k1/first;
		rem  = k1%first;
#ifndef DEBUG		
		printf("\n---------Thread: %d First Paritioning: k:%d group:%d rem:%d first: %d last: %d------------\n", thread_id, k1, group1, rem1, first, last);
		printf("loop from: %d to %d\n", barr_id, barr_id+first);
#endif	
		cnt1 = barr_id + first;
		for(i = barr_id, s = 0, e = ps_msg[thread_id].start-1; i < cnt1; i++) {
			s = e + 1;
			e = s + group - 1;
			if(rem > 0) {
				rem--;
				e++;
			}
			ps_msg[i].pstart = ps_msg[thread_id].pstart;
			ps_msg[i].pend   = pivots_indices[barr_id]-1;
			ps_msg[i].start  = s;
			ps_msg[i].end    = e;
			ps_msg[i].first_thread = barr_id;
			ps_msg[i].last_thread  = cnt1-1;
		}

#ifndef DEBUG
		printf("\n---------Thread: %d Second Paritioning: k:%d group:%d rem:%d first: %d last: %d------------\n", thread_id, k2, group2, rem2, first, last);
		printf("loop from: %d to %d\n",  barr_id+first,barr_id+first+last);
#endif
	
		cnt1 += last;		
		group = k2/last;
		rem   = k2%last;

		for(i = (barr_id+first), e = pivots_indices[barr_id]-1; i < cnt1; i++) {
			s = e + 1;
			e = s + group - 1;
			if(rem > 0) {
				rem--;
				e++;
			}
			ps_msg[i].pstart = pivots_indices[barr_id];
			ps_msg[i].pend   = prev_end;
			ps_msg[i].start = s;
			ps_msg[i].end   = e;
			ps_msg[i].first_thread = barr_id+first;
			ps_msg[i].last_thread  = cnt1-1;
		}

#ifndef DEBUG
		for(i = 0; i < num_threads; i++) {
			printf("Thread id: %d pstart: %d pend: %d start: %d end: %d first_thread:%d last_thread: %d\n", ps_msg[i].thread_id, ps_msg[i].pstart, ps_msg[i].pend, ps_msg[i].start, ps_msg[i].end, ps_msg[i].first_thread, ps_msg[i].last_thread);
		}
		printf("cbarr: %d %d\n",barr_id, first);
		printf("cbarr: %d %d\n",barr_id+first, last);
#endif
		pthread_barrier_destroy(&cbarr[barr_id]);
		pthread_barrier_init(&cbarr[barr_id], NULL, first);
		pthread_barrier_destroy(&cbarr[barr_id+first]);
		
		pthread_barrier_init(&cbarr[barr_id+first], NULL, last);
		for(i = barr_id+1; i <= prev_lt; i++) {
			pthread_barrier_wait(&obarr[i]);
		}
		//printf("Thread id: %d Master thread done !!\n", thread_id);
	} else {
		pthread_barrier_wait(&obarr[thread_id]);
		//printf("Thread %d done with a level\n",thread_id);
	  }
		depth++;
	}
}



void spawn_threads(const int len) {
	int group = len / num_threads;
	int rem   = len % num_threads;
	int i, s = 0, e = -1;

	for(i = 0; i < num_threads; i++) {
		pthread_barrier_init(&cbarr[i], NULL, num_threads);
		pthread_barrier_init(&obarr[i], NULL, 2);
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
		ps_msg[i].last_thread = num_threads-1;
		ps_msg[i].first_thread = 0;
		if(pthread_create(&threads[i], NULL, calcPrefixSum, 
				        (void *)& ps_msg[i].thread_id)== -1) {
			printf("prefixSum: pthread_create\n",__func__);
			exit(EXIT_FAILURE);
		}
	}
	for(i = 0; i < num_threads; i++) {
		pthread_join(threads[i], NULL);
	}
}

void validate(int* output, int num_elements) {
  int i;
  for(i = 0; i < num_elements - 1; i++) {
    if (output[i] > output[i+1]) {
      printf("************* NOT sorted *************\n");
      return;
    }
  }
  printf("============= SORTED ===========\n"); 
}


int main(int argc, char** argv) { 
    if(argc != 2) {
	  printf("Usage: ./pqsort <num of threads>\n");
	  return EXIT_FAILURE;
	}

   num_threads = atoi(argv[1]);
   int i, s = 0, e = -1, group = INPUT_SIZE / num_threads, rem = INPUT_SIZE% num_threads;
   double sTime, pTime;
   struct timeval tz;
   struct timezone tx;
   double start_time, end_time;
 
   if( ((cbarr = (pthread_barrier_t*)malloc(sizeof(pthread_barrier_t)*num_threads)) == NULL) ||
  	((obarr = (pthread_barrier_t*)malloc(sizeof(pthread_barrier_t)*num_threads)) == NULL) ||
  	((threads = (pthread_t*)malloc(sizeof(pthread_t)*num_threads)) == NULL) ||
   	((ps_msg = (prefixSumMsg*)malloc(sizeof(prefixSumMsg)*num_threads)) == NULL) ||
   	((sum1 = (int *)malloc(sizeof(int)*num_threads)) == NULL) ||
   	((sum2 = (int *)malloc(sizeof(int)*num_threads)) == NULL) ||
 	((medians = (int *)malloc(sizeof(int)*num_threads)) == NULL) ||
  	((pivots = (int *)malloc(sizeof(int)*num_threads)) == NULL) ||
  	((pivots_indices = (int *)malloc(sizeof(int)*num_threads)) == NULL) ||
	((input1 = (int*)malloc(sizeof(int)*INPUT_SIZE)) == NULL) ||
 	((input2 = (int*)malloc(sizeof(int)*INPUT_SIZE)) == NULL) ) {
	exit(EXIT_FAILURE);
}

    srand(time(NULL));
    for(i = 0; i < INPUT_SIZE; i++) 
	input1[i] = rand()%INPUT_SIZE;

    gettimeofday(&tz, &tx);
    start_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;

  //  spawn_threads(INPUT_SIZE);

	for(i = 0; i < num_threads; i++) {
		pthread_barrier_init(&cbarr[i], NULL, num_threads);
		pthread_barrier_init(&obarr[i], NULL, 2);
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
		ps_msg[i].pend   = INPUT_SIZE-1;
		ps_msg[i].last_thread = num_threads-1;
		ps_msg[i].first_thread = 0;
		if(pthread_create(&threads[i], NULL, calcPrefixSum, 
				        (void *)& ps_msg[i].thread_id)== -1) {
			printf("prefixSum: pthread_create\n",__func__);
			return EXIT_FAILURE;
		}
	}

	for(i = 0; i < num_threads; i++) {
		pthread_join(threads[i], NULL);
	}

    gettimeofday(&tz, &tx);
    end_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;
    pTime = end_time-start_time;
    printf("Parallel Time: time_p %lf\n", pTime);
    printf("Speedup: %lf\n", 4/pTime);
    validate(input1, INPUT_SIZE);

/*  for(i=0;i<INPUT_SIZE;i++){
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
			cout<<"\n";
			cout<<"Expected answer:\n";
			for(int k = 0; k < INPUT_SIZE; k++)
					cout<<check[k]<<", ";
			  break;
			}
    }
    printf("\n");
*/
    return EXIT_SUCCESS;
}
