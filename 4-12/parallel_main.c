#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "utilities.h"

#define N 10000000000

static inline
double f (long long i) {
    double x;
    x =  (double) i / (double) N;
    return 4.0 / (1.0 + x * x);
}

int main (int argc, char *argv[]) {
    double local_area = 0;
	double global_area;
    long long i;
	int size, rank;
	double elapse_time;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	if (!rank) elapse_time = -MPI_Wtime();

    if(!rank) global_area = f(0) - f(N);

    for (i = rank + 1; i <= N / 2; i += size) local_area += 4.0 * f(2 * i - 1) + 2 * f(2 * i);
	
	MPI_Reduce(&local_area, &global_area, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (!rank) {
		global_area /= (3.0 * N);
		elapse_time += MPI_Wtime();
    	printf ("Approximation of pi: %13.11f\n", global_area);
		output_real_exec_time(elapse_time);
		//printf("elapse time : %lf\n", elapse_time);
	}

	MPI_Finalize();
    exit(0);
}

