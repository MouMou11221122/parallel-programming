#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>
#include <math.h>

#include "utilities.h"
#include "header.h"


long long* global_min_info = NULL,* global_max_info = NULL;

void diff(long long* l_max, long long pre, long long cur) {
	*l_max = *l_max <= (cur - pre) ? (cur - pre) : *l_max;
}

void cal_result (long long g_max, long long* g_min_info, long long* g_max_info, int p) {
	int pre_idx, cur_idx;
	long long pre_val, cur_val;
	pre_idx = 0;
	cur_idx = 1;
	pre_val = g_max_info[pre_idx];
	cur_val = g_min_info[cur_idx];

	while (cur_idx <= p) {
		if (pre_val == -1 && cur_val == -1) {
			pre_idx = cur_idx;
			++cur_idx;
			pre_val = g_max_info[pre_idx];
			if (cur_idx <= p) {
				cur_val = g_min_info[cur_idx];
			} else	break;
		} else if (pre_val != -1 && cur_val == -1) {
			++cur_idx;
			if (cur_idx <= p) {
				cur_val = g_min_info[cur_idx];
			} else break;
		} else if (pre_val == -1 && cur_val != -1) {
			pre_idx = cur_idx;
			pre_val = g_max_info[pre_idx];
			++cur_idx;
			if (cur_idx <=p) {
				cur_val = g_min_info[cur_idx];
			} else break;
		} else {
			long long diff = cur_val - pre_val;
			if(diff > g_max)	g_max = diff;
			pre_idx = cur_idx;
			pre_val = g_max_info[pre_idx];
			++cur_idx;
			if (cur_idx <= p) {
				cur_val = g_min_info[cur_idx];
			} else	break;
		}
	}
	
	printf("result : %lld\n", g_max);
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
	long long cnt;
	long long global_cnt = 0;
	long long local_max = -1;
	long long global_max = 0;
	long long local_min_prime = -1, local_max_prime = -1;
	long long pre, cur;
	long long* local_max_ary = NULL;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &idx);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	n = atoll(argv[1]);

	if(!idx) {
		global_min_info = malloc(sizeof(long long) * p);
		global_max_info = malloc(sizeof(long long) * p);
		//remember to free
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

	for(long long i = 0; i < size; ++i) {
		if(!marked[i]) {
			if(local_min_prime == -1) {
				cur = local_min_prime = local_max_prime = low_val + i;
			} else {
				pre = cur;
				cur = local_max_prime = low_val + i;
				diff(&local_max, pre, cur);
			}
		}
	}

	MPI_Reduce(&local_max, &global_max, 1, MPI_LONG_LONG, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Gather(&local_min_prime, 1, MPI_LONG_LONG, global_min_info + idx, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
	MPI_Gather(&local_max_prime, 1, MPI_LONG_LONG, global_max_info + idx, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
	if(!idx) {
		cal_result(global_max, global_min_info, global_max_info, p - 1);
		free(global_min_info);
		free(global_max_info);
	}
	free(marked);
	MPI_Finalize();
	exit(0);
}
