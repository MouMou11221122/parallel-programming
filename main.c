#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "utilities.h"

int main(int argc, char** argv) {
	double start_time, end_time, real_start_time, real_end_time;
	MPI_Init(&argc, &argv);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	start_time = MPI_Wtime();
	printf("Hello World from rank %d process\n", rank);
	end_time = MPI_Wtime();

	MPI_Reduce(&start_time, &real_start_time, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
	MPI_Reduce(&end_time, &real_end_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	printf("start time : %lf	end time : %lf from process %d\n", start_time, end_time, rank);

	if(!rank) {
		printf("real start time : %lf	real end time : %lf\n", real_start_time, real_end_time);
		output_real_exec_time(real_end_time - real_start_time);
	}

	MPI_Finalize();
	exit(0);
}
