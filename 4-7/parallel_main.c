#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "utilities.h"

int main(int argc, char** argv) {
	long long n;
	int idx, p;
	double start_time, end_time;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s [n]\n", argv[0]);
		exit(1);
	}

	n = atoll(argv[1]);

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &idx);
	MPI_Comm_size(MPI_COMM_WORLD, &p);


	MPI_Barrier(MPI_COMM_WORLD);
	start_time = MPI_Wtime();

	long long global_count = 0;
	long long local_count = 0;

	for (long long i = idx + 1; i <= n; i += p) {
		local_count += i;
	}
	
	printf("local count : %lld\n", local_count);

	MPI_Reduce(&local_count, &global_count, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

	if(!idx) {
		end_time = MPI_Wtime();
		output_real_exec_time(end_time - start_time);
		printf("result : %lld\n", global_count);
	}

	MPI_Finalize();
	exit(0);
}
