#include <stdio.h>
#include <gmp.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <math.h>
#include "utilities.h"

#define DIGITS(n)  ((n) == 0 ? 1 : (int)(log10((n) < 0 ? -(n) : (n))) + 1)

char *local_buf;

char output_format[16];
int *recv_cnt;
char *recv_buf;
int *displsmnt;
int logn_ceil;
int digits;

int main(int argc, char** argv) {
    mpf_t pre_result;
    mpf_t pre_inv;
    mpf_t pre_one;
	mpf_t pre_i;
	mpz_t pre_i_str;
	int rank, size;
	double start_time, end_time;
	double real_start_time, real_end_time;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

	start_time = MPI_Wtime();

    mpf_set_default_prec(512);  

	long long n = atoll(argv[1]);
	int prec = atoi(argv[2]);
	snprintf(output_format, sizeof(output_format), "%%.%dFf\n", prec);
	logn_ceil = (int)(ceil(log((double)n)));
	digits = logn_ceil + prec + 64;

	if (!rank) {
		recv_cnt = malloc(sizeof(int) * size);
		for (int i = 0; i < size; i++) recv_cnt[i] = digits;

		displsmnt = malloc(sizeof(int) * size);
		for (int i = 0; i < size; i++) displsmnt[i] = i * digits;

		recv_buf = malloc(digits * size);
	}

    mpf_init(pre_result);  
    mpf_init(pre_inv);  
    mpf_init(pre_one);  
    mpf_init(pre_i);  
    mpz_init(pre_i_str);  

    mpf_set_ui(pre_result, 0);  
    mpf_set_ui(pre_one, 1);  

	char str[32];
	for (long long i = rank + 1; i <= n; i += size) {
		sprintf(str, "%lld", i);
		mpz_set_str(pre_i_str, str, 10);
		mpf_set_z(pre_i, pre_i_str);
		mpf_div(pre_inv, pre_one, pre_i);
		mpf_add(pre_result, pre_inv, pre_result);
	}

	if (!rank) local_buf = recv_buf;
	else local_buf = malloc(digits);

	gmp_sprintf(local_buf, output_format, pre_result);

	MPI_Gatherv(local_buf, digits, MPI_CHAR, recv_buf, recv_cnt, displsmnt, MPI_CHAR, 0, MPI_COMM_WORLD);
	if (rank) end_time = MPI_Wtime();
	if (!rank) {
		mpf_t tmp_sum, global_sum;
		mpf_init(tmp_sum);
		mpf_init(global_sum);
		mpf_set_ui(global_sum, 0);

		for (int i = 0; i < size; i++) {
			gmp_sscanf(recv_buf + i * digits, "%Ff", tmp_sum);
			mpf_add(global_sum, global_sum, tmp_sum);
		}

		end_time = MPI_Wtime();

		gmp_printf(output_format, global_sum);
    	mpf_clear(tmp_sum); 
    	mpf_clear(global_sum); 
	}

	MPI_Reduce(&start_time, &real_start_time, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
	MPI_Reduce(&end_time, &real_end_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	if (!rank) {
		printf("real execution time : %.10f\n", real_end_time - real_start_time);
		output_real_exec_time(real_end_time - real_start_time);
		free(recv_cnt);
		free(displsmnt);
		free(recv_buf);
	}
    mpf_clear(pre_result); 
    mpf_clear(pre_inv); 
    mpf_clear(pre_one); 
    mpf_clear(pre_i); 
	MPI_Finalize();
    exit(0);
}

