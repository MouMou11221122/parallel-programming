#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "utilities.h"

void sum(int rank, int size, long long* partial_sum, long long boundary) {
	for(long long i = rank + 1; i <= boundary; i += size) *partial_sum += i;
}

 
void correctness(long long global_sum, long long boundary) {
	long long correct_sum = 0;
	correct_sum = boundary * (1 + boundary) / 2;
	if (correct_sum != global_sum) {
		fprintf(stderr, "computation error\n");
		exit(1);	
	}
	printf("result is %lld\n", correct_sum);		
}




int main(int argc, char** argv) {
	double start_time, end_time;
	long long p;
	long long global_sum = 0, partial_sum = 0;

	p = atoll(argv[1]);

	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	start_time = MPI_Wtime();
	sum(rank, size, &partial_sum, p);	

	MPI_Reduce(&partial_sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
	
	if(!rank) {
		correctness(global_sum, p);
		end_time = MPI_Wtime();
		output_real_exec_time(end_time - start_time);
	}

	MPI_Finalize();
	exit(0);
}
