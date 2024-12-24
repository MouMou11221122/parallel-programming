#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <signal.h>
#include "utilities.h"

#define BATCH_SIZE		300
#define JOB_ASSIGN		1
#define RESULT_REPLY	2


int interval[2];
MPI_Request * send_rqst;
MPI_Request * recv_rqst;
MPI_Status * send_stat;	
MPI_Status recv_stat;
MPI_Status tmp_stat;
MPI_Status probe_stat;
int * result_buf;
pid_t pid;
bool * busy;
int send_ptr;
int recv_ptr;

bool isPrime(int num) {
    if (num <= 1) return false;
    if (num <= 3) return true; 
    if (num % 2 == 0 || num % 3 == 0) return false;
    for (int i = 5; i * i <= num; i += 6) {
        if (num % i == 0 || num % (i + 2) == 0) return false;
    }
    return true;
}


bool isPerfectNumber(int n) {
    if (n <= 1) return false;

    //assume n = 2^(p-1) * (2^p - 1)
    for (int p = 2; (1 << (p - 1)) * ((1 << p) - 1) <= n; p++) {
        int mersennePrime = (1 << p) - 1; 
        if (isPrime(mersennePrime)) {
            int perfectNumber = (1 << (p - 1)) * mersennePrime;
            if (perfectNumber == n) {
                return true; 
            }
        }
    }
    return false; 
}

/*
bool isPerfectNumber(int n) {
	if (n <= 1) return false;
	int sum = 1;
	for (int i = 2; i * i <= n; i++) {
		if (n % i == 0) {
			sum += i;
			if (i != n / i) sum += n / i;
		}
	}
	return sum == n;
}
*/

void findRangeResult(int * buf) {
	int cnt = 0;
	for (int i = interval[0]; i <= interval[1]; i++) {
		if (isPerfectNumber(i)) {
			cnt++;
			buf[cnt] = i;
		}
	}
	if (cnt == 0) buf[0] = -1;
	else buf[0] = cnt;
}

void signal_handler(int signum) {
    //printf("Received signal %d from pid:%d...\n", signum, getpid());
	//MPI_Finalize();
    exit(0);  
}


int main (int argc, char* argv[]) {
	MPI_Init(&argc, &argv);
	int rank, p;
	int n;
	int cnt = 0;
	double start_time, end_time;

	n = atoi(argv[1]);
	pid = getpid();

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p); 
	int * tmp_buf;
	tmp_buf = malloc(sizeof(int) * (n + 1));

	if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("Unable to catch SIGINT");
        exit(1);
    }

	if (p == 1) {
		int i = 1;
		printf("first n perfect number:\n");
		while (cnt != n) {
			if (isPerfectNumber(i)) {
				printf("%d ", i);
				cnt++;
			}
			i++;
		}
		printf("\n");
		MPI_Finalize();
		exit(0);
	}

	if (!rank) {
		//start from send_ptr = recv_ptr = 1
		int batch_cnt = 0;
		pid_t * pid_arr;
		
		pid_arr = malloc(sizeof(pid_t) * p);
		result_buf = malloc(sizeof(int) * n);
		busy = malloc(sizeof(bool) * (p - 1));
		send_rqst = malloc(sizeof(MPI_Request) * (p-1));
		send_stat = malloc(sizeof(MPI_Status) * (p-1));

		MPI_Gather(&pid, 1, MPI_INT, pid_arr, 1, MPI_INT, 0, MPI_COMM_WORLD);

		for (int i = 0; i < p - 1; i++) busy[i] = false;

		start_time = MPI_Wtime();
		while (cnt != n) {
			//cyclic assign send request to non-busy processes
			while (!busy[send_ptr]) {
				busy[send_ptr] = true;
				interval[0] = batch_cnt * BATCH_SIZE;
				interval[1] = interval[0] + BATCH_SIZE - 1;
				batch_cnt++;
				MPI_Isend(interval, 2, MPI_INT, send_ptr + 1, JOB_ASSIGN, MPI_COMM_WORLD, send_rqst + send_ptr);
				MPI_Wait(send_rqst + send_ptr, send_stat + send_ptr);
				send_ptr = (send_ptr + 1) % (p - 1);
			}
			
			//cyclic get the result from the process who finishes their job
			//every time receive a result reply the master process initial send requests to non-busy processes
			MPI_Recv(tmp_buf, n + 1, MPI_INT, recv_ptr + 1, RESULT_REPLY, MPI_COMM_WORLD, &tmp_stat);
			if (tmp_buf[0] != -1) {
				for (int i = 1; i <= tmp_buf[0]; i++) {
					result_buf[cnt] = tmp_buf[i];
					cnt++;
				}
			}
			busy[recv_ptr] = false; 
			recv_ptr = (recv_ptr + 1) % (p - 1);
		}
		
		end_time = MPI_Wtime();

		printf("first n perfect number:\n");
		for (int i = 0; i < n; i++) printf("%d ", result_buf[i]);
		printf("\n");
		
		output_real_exec_time(end_time - start_time);

		for(int i = 1; i < p; i++) {
			if (kill(pid_arr[i], SIGINT) == -1) {
        		perror("Error sending SIGINT from master process");
        		exit(1);
    		}
		}

	} else {
		MPI_Status slave_recv_stat;
		MPI_Gather(&pid, 1, MPI_INT, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD);
		while (1) {
			MPI_Recv(interval, 2, MPI_INT, 0, JOB_ASSIGN, MPI_COMM_WORLD, &slave_recv_stat);
			findRangeResult(tmp_buf);
			MPI_Send(tmp_buf, n + 1, MPI_INT, 0, RESULT_REPLY, MPI_COMM_WORLD);
		}
	}
	

	MPI_Finalize();
	exit(0);
}

