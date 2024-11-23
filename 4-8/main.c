#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>
#include <math.h>

#include "utilities.h"
#include "header.h"


long long* first_prime, * last_prime;

void cal_result (int p, long long* g_cnt) {
	long long cnt = 0;
	int pre, cur;
	long long pre_val, cur_val;
	pre = 0;
	cur = 1;
	pre_val = last_prime[0];
	cur_val = first_prime[1];
	while (cur < p) {
		if (pre_val != -1 && cur_val != -1) {
			if (cur_val - pre_val == 2) ++cnt;
			pre = cur;
			pre_val = last_prime[pre];
			++cur;
			if (cur < p) cur_val = first_prime[cur];
			else break;
		} else if (pre_val != -1 && cur_val == -1) {
			++cur;
			if (cur < p) cur_val = first_prime[cur];
			else break;
		} else {
			pre = cur;
			pre_val = last_prime[pre];
			++cur;
			if (cur < p) cur_val = first_prime[cur];
			else break;
		}
	}
	*g_cnt += cnt;
	printf("result : %lld\n", *g_cnt);
}



int main(int argc, char** argv) {
	int idx, p;
	long long n, size;
	long long low_val, high_val;
	long long proc0_size;
	bool* marked;
	long long prime;
	long long first;
	long long global_idx;
	double start_time, end_time;



	if (argc != 2) {
		fprintf(stderr, "Usage: %s [n]\n", argv[0]);
		exit(1);
	}

	n = atoll(argv[1]);

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &idx);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	start_time = MPI_Wtime();

	if(!idx) {
		first_prime = malloc(sizeof(long long) * p);
		if (!first_prime) {
			fprintf(stderr, "Memory allocation fail\n");
			exit(1);
		}
		last_prime = malloc(sizeof(long long) * p);
		if (!last_prime) {
			fprintf(stderr, "Memory allocation fail\n");
			exit(1);
		}
	}

	low_val = 2 + BLOCK_LOW(idx, p, n - 1);
	high_val = 2 + BLOCK_HIGH(idx, p, n - 1);
	size = BLOCK_SIZE(idx, p, n - 1);

	proc0_size = (n - 1) / p - 1;
	if((2 + proc0_size) < (long long)sqrt((double)n)) {
		if(!idx) fprintf(stderr, "Too many process\n"); 
		MPI_Finalize();
		exit(1);
	}

	marked = (bool *) malloc(size);

	if(marked == NULL) {
		fprintf(stderr, "Memory allocation failed from process %d\n", idx);
		MPI_Finalize();
		exit(1);
	}

	for(long long i = 0; i < size; ++i) marked[i] = false;

	if(!idx) global_idx = 0;
	prime = 2;
	do {
		if(prime * prime > low_val)	first = prime * prime - low_val;
		else {
			if(!(low_val % prime)) first = 0;
			else	first = prime - (low_val % prime);
		}
		for(long long i = first; i < size; i+= prime)	marked[i] = true;
		if(!idx) {
			while(marked[++global_idx]);
			prime = global_idx + 2;
		}
		MPI_Bcast(&prime, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
	} while (prime * prime <= n);

	MPI_Barrier(MPI_COMM_WORLD);
	
	long long local_count = 0, global_count;
	long long pre, cur;
	long long _first = -1, _last = -1;

	for(long long i = 0; i < size; ++i) {
		if (!marked[i]) {
			if (_first == -1) {
				_first = _last = low_val + i;
				cur = _first;
			} else {
				_last = low_val + i;
				pre = cur;
				cur = _last;
				if (cur - pre == 2) ++local_count;
			}
		}
	}

	MPI_Reduce(&local_count, &global_count, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_Gather(&_first, 1, MPI_LONG_LONG, first_prime + idx, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
	MPI_Gather(&_last, 1, MPI_LONG_LONG, last_prime + idx, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

	if(!idx) {
		cal_result(p, &global_count);
		end_time = MPI_Wtime();
		output_real_exec_time(end_time - start_time);
		free(first_prime);
		free(last_prime);
	}

	free(marked);
	MPI_Finalize();
	exit(0);
}
