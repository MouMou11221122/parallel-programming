#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define LOOP_TIMES 128
#define FIRST_SIZE	(1 << 21)
#define SECOND_SIZE	(1 << 26)


static inline 
void check_err(int err) {
	if (err != MPI_SUCCESS) {
		fprintf(stderr, "MPI_Send failed\n");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
}


void solve_equation(double i, double j, double c1, double c2, double *x, double *y) {
    if (c1 == c2) {
        printf("Error: c1 and c2 cannot be equal, as it would lead to division by zero.\n");
        exit(1);
    }
    *y = (i - j) / (c1 - c2);
    *x = c1 - i / *y;
}


int main (int argc, char** argv) {
	int rank;
	double first_elapse_time;
	double second_elapse_time;
	
	double avg_first_elapse_time;
	double avg_second_elapse_time;

	int loop_cnt;
	char *buf;


	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	buf = malloc(sizeof(char) * FIRST_SIZE);
	if (!buf) {
		fprintf(stderr, "Memory allocation error.\n");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	loop_cnt = 0;
	if(!rank) first_elapse_time = -MPI_Wtime();
	while (loop_cnt < LOOP_TIMES) {
		int err;
		if (!rank) {
			err = MPI_Send(buf, FIRST_SIZE, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
			check_err(err);

			err = MPI_Recv(buf, FIRST_SIZE, MPI_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			check_err(err);
		} else {
			err = MPI_Recv(buf, FIRST_SIZE, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			check_err(err);

			err = MPI_Send(buf, FIRST_SIZE, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
			check_err(err);
		}
		loop_cnt++;
	}
	if(!rank) {
		first_elapse_time += MPI_Wtime();
		avg_first_elapse_time = first_elapse_time / (double)LOOP_TIMES;
		printf("first average elapse time : %lf\n", avg_first_elapse_time);
	}
	free(buf);


	buf = malloc(sizeof(char) * SECOND_SIZE);
	if (!buf) {
		fprintf(stderr, "Memory allocation error.\n");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	loop_cnt = 0;
	second_elapse_time = -MPI_Wtime();
	while (loop_cnt < LOOP_TIMES) {
		int err;
		if (!rank) {
			err = MPI_Send(buf, SECOND_SIZE, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
			check_err(err);

			err = MPI_Recv(buf, SECOND_SIZE, MPI_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			check_err(err);
		} else {
			err = MPI_Recv(buf, SECOND_SIZE, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			check_err(err);

			err = MPI_Send(buf, SECOND_SIZE, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
			check_err(err);
		}
		loop_cnt++;
	}
	if(!rank) {
		second_elapse_time += MPI_Wtime();
		avg_second_elapse_time = second_elapse_time / (double)LOOP_TIMES;
		printf("second average elapse time : %lf\n", avg_second_elapse_time);
		//lamda + FIRST_SIZE / beta = avg_first_elapse_time;
		//lamda + SECOND_SIZE / beta = avg_second_elapse_time;
		double lamda, beta;
		solve_equation(FIRST_SIZE, SECOND_SIZE, avg_first_elapse_time, avg_second_elapse_time, &lamda, &beta);
		printf("lamda : %lf	beta : %lf\n", lamda, beta);
	}
	free(buf);

	MPI_Finalize();
	exit(0);
}






