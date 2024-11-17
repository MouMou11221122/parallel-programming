#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <mpi.h>

#define BLOCK_LOW(id, p, n)  ((id) * (n) / (p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id) + 1, p, n) - 1)
#define BLOCK_NUMS(id, p, n) (BLOCK_LOW((id) + 1, p, n) - BLOCK_LOW(id, p, n)) 
#define DOWNSTREAM_MSG	1
#define UPSTREAM_MSG	2
#define DUMMY_MSG		3
#define OUTPUT_MSG		4

int proc_num;
int	proc_idx;
int row_num;
int col_num;

int blk_num;
int cell_num;
int file_offset;


void show_sub_matrix (bool* sub_matrix) {
	if (proc_idx == 0) {
		//printf("sub-matrix from process %d :\n", proc_idx);
		for (int i = 0; i < blk_num; i++) {
			for(int j = 0; j < col_num; j++) {
				printf("%d ", sub_matrix[i * col_num + j]);
			}
			printf("\n");
		}
	} else {
		MPI_Send(sub_matrix, cell_num, MPI_CHAR, 0, OUTPUT_MSG, MPI_COMM_WORLD);
	}
	//fflush(NULL);
}

void update_row (bool* top_row, bool* middle_row, bool* bottom_row, bool* updated_sub_matrix, int updated_row_idx)
{
	for (int idx = 0; idx < col_num; idx++){
		char life_cnt = 0;
		/*	top row	*/
		if (top_row) {
			if (idx > 0) life_cnt += top_row[idx - 1];
			life_cnt += top_row[idx];
			if (idx < col_num - 1) life_cnt += top_row[idx + 1];
		}

		/*	middle row	*/
		if (idx > 0) life_cnt += middle_row[idx - 1];
		if (idx < col_num - 1) life_cnt += middle_row[idx + 1];

		/*	bottom row	*/
		if (bottom_row) {
			if (idx > 0) life_cnt += bottom_row[idx - 1];
			life_cnt += bottom_row[idx];
			if (idx < col_num - 1) life_cnt += bottom_row[idx + 1];
		}
		
		/*	debugging	*/
		//printf("Updating row %d col %d of process %d.........., life number = %d\n", updated_row_idx, idx, proc_idx, life_cnt);
		/*	debugging	*/

		if (middle_row[idx]) {
			if (life_cnt != 2 && life_cnt != 3)	updated_sub_matrix[updated_row_idx * col_num + idx] = false;
			else updated_sub_matrix[updated_row_idx * col_num + idx] = true; 
		} else {
			if (life_cnt == 3) updated_sub_matrix[updated_row_idx * col_num + idx] = true;
			else updated_sub_matrix[updated_row_idx * col_num + idx] = false;
		}
	}
}


void update_sub_matrix (bool** sub_matrix) 
{
	int first_row = 0;
	int last_row = blk_num - 1;

	bool* first_row_recv_buf;
	bool* last_row_recv_buf;

	first_row_recv_buf = malloc(sizeof(bool) * col_num);
	if (first_row_recv_buf == NULL) {
		fprintf(stderr, "fail to allocate receive first row buffer memory from process %d\n", proc_idx);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	last_row_recv_buf = malloc(sizeof(bool) * col_num);
	if (last_row_recv_buf == NULL) {
		fprintf(stderr, "fail to allocate receive last row buffer memory from process %d\n", proc_idx);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	bool* updated_sub_matrix;
	updated_sub_matrix = malloc(sizeof(bool) * cell_num);
	if (updated_sub_matrix == NULL) {
		fprintf(stderr, "fail to allocate updated sub-matrix memory from process %d\n", proc_idx);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	if (proc_idx == 0) {
		MPI_Send(*sub_matrix + last_row * col_num, col_num, MPI_CHAR, proc_idx + 1, DOWNSTREAM_MSG, MPI_COMM_WORLD);
		MPI_Recv(last_row_recv_buf, col_num, MPI_CHAR, proc_idx + 1, UPSTREAM_MSG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		free(first_row_recv_buf);
		first_row_recv_buf = NULL;
	} else if (proc_idx == proc_num - 1) {
		MPI_Send(*sub_matrix + first_row * col_num, col_num, MPI_CHAR, proc_idx - 1, UPSTREAM_MSG, MPI_COMM_WORLD);
		MPI_Recv(first_row_recv_buf, col_num, MPI_CHAR, proc_idx - 1, DOWNSTREAM_MSG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		free(last_row_recv_buf);
		last_row_recv_buf = NULL;
	} else {
		MPI_Send(*sub_matrix + last_row * col_num, col_num, MPI_CHAR, proc_idx + 1, DOWNSTREAM_MSG, MPI_COMM_WORLD);
		MPI_Send(*sub_matrix + first_row * col_num, col_num, MPI_CHAR, proc_idx - 1, UPSTREAM_MSG, MPI_COMM_WORLD);
		MPI_Recv(first_row_recv_buf, col_num, MPI_CHAR, proc_idx - 1, DOWNSTREAM_MSG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(last_row_recv_buf, col_num, MPI_CHAR, proc_idx + 1, UPSTREAM_MSG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}

	/*	debugging	*/
	/*
	if (first_row_recv_buf) {
		printf("first row of process %d :\n", proc_idx);
		for (int i = 0; i < col_num; i++) {
			printf("%d ", first_row_recv_buf[i]);
		}
		printf("\n");
	} else {
		printf("first row of process %d : NULL\n", proc_idx);
	}
	
	if (last_row_recv_buf) {
		printf("last row of process %d :\n", proc_idx);
		for (int i = 0; i < col_num; i++) {
			printf("%d ", last_row_recv_buf[i]);
		}
		printf("\n");
	} else {
		printf("last row of process %d : NULL\n", proc_idx);
	}
	*/
	/*	debugging	*/


	for (int i = first_row; i <= last_row; i++) {
		if (i == first_row && i == last_row) {
			update_row(first_row_recv_buf, *sub_matrix + i * col_num, last_row_recv_buf, updated_sub_matrix, i);
		} else if (i == first_row && i != last_row) {
			update_row(first_row_recv_buf, *sub_matrix + i * col_num, *sub_matrix + (i + 1) * col_num, updated_sub_matrix, i);
		} else if (i != first_row && i == last_row) {
			update_row(*sub_matrix + (i - 1) * col_num, *sub_matrix + i * col_num, last_row_recv_buf, updated_sub_matrix, i);
		} else {
			update_row(*sub_matrix + (i - 1) * col_num, *sub_matrix + i * col_num, *sub_matrix + (i + 1) * col_num, updated_sub_matrix, i);
		}
	}
	free(*sub_matrix);
	*sub_matrix = updated_sub_matrix;
	if (first_row_recv_buf) free(first_row_recv_buf);
	if (last_row_recv_buf) free(last_row_recv_buf);
}

void periolically_show (bool** sub_matrix, int iteration, int freq) {
	char dummy;
	int cnt = 0;
	bool* output_buf;

	if (proc_idx == 0) output_buf = malloc(sizeof(bool) * BLOCK_NUMS(proc_num - 1, proc_num, row_num));

	for (int i = -1; i < iteration; i++) {
		if (i == -1 || ++cnt == freq) {
			if (proc_idx == 0) {
				printf("iteration %d :\n", i + 1);
				show_sub_matrix(*sub_matrix);
				//fflush(NULL);
				MPI_Send(&dummy, 1, MPI_CHAR, proc_idx + 1, DUMMY_MSG, MPI_COMM_WORLD);
				for (int i = 1; i < proc_num; i++ ) {
					MPI_Recv(output_buf, BLOCK_NUMS(i, proc_num, row_num) * col_num, MPI_CHAR, i, OUTPUT_MSG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					//printf("sub-matrix from process %d :\n", i);
					for (int j = 0; j < BLOCK_NUMS(i, proc_num, row_num); j++) {
						for (int k = 0; k < col_num; k++) {
							printf("%d ", output_buf[j * col_num + k]);
						}
						printf("\n");
					}
				}
			} else if (proc_idx == proc_num - 1) {
				MPI_Recv(&dummy, 1, MPI_CHAR, proc_idx - 1, DUMMY_MSG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				show_sub_matrix(*sub_matrix);
				//fflush(NULL);
			} else {
				MPI_Recv(&dummy, 1, MPI_CHAR, proc_idx - 1, DUMMY_MSG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				show_sub_matrix(*sub_matrix);
				//fflush(NULL);
				MPI_Send(&dummy, 1, MPI_CHAR, proc_idx + 1, DUMMY_MSG, MPI_COMM_WORLD);	
			}
			cnt = 0;
			MPI_Barrier(MPI_COMM_WORLD);
		}
		if (i != iteration - 1) update_sub_matrix(sub_matrix);
	}

	if (proc_idx == 0) free(output_buf);
}



int main (int argc, char** argv) {
	int fd;
	char* file_name;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &proc_idx);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_num);

	if (!proc_idx && argc != 2) {
		fprintf(stderr, "Usage: %s [file name]\n", argv[0]);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	file_name = argv[1];
	MPI_Barrier(MPI_COMM_WORLD);

	/*	open matrix.bin	*/
	fd = open(file_name, O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "Fail to open file : matrix.bin from process %d\n", proc_idx);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	
	ssize_t read_num;
	/*	read row nums	*/
	if ((read_num = read(fd, &row_num, sizeof(int))) == -1) {
		fprintf(stderr, "Read row error from process %d\n", proc_idx);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	/*	read column nums	*/
	if ((read_num = read(fd, &col_num, sizeof(int))) == -1) {
		fprintf(stderr, "Read column error from process %d\n", proc_idx);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	/*	Calculate the local block num and local cell num	*/
	/*	and allocate the sub-matrix for cells. Then read	*/
	/*	the cells from matrix.bin.							*/
	bool* sub_matrix;
	blk_num = BLOCK_NUMS(proc_idx, proc_num, row_num);
	cell_num = blk_num * col_num;
	sub_matrix = malloc(sizeof(bool) * cell_num);
	if (sub_matrix == NULL) {
		fprintf(stderr, "Fail to allocate sub-matrix memory space from process %d\n", proc_idx);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	file_offset = BLOCK_LOW(proc_idx, proc_num, row_num) * col_num * sizeof(bool);
	lseek(fd, file_offset, SEEK_CUR);
	if ((read_num = read(fd, sub_matrix, cell_num * sizeof(bool))) == -1 || read_num != cell_num * sizeof(bool)) {
		fprintf(stderr, "Read sub-matrix error from process %d\n", proc_idx);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	

	/*	Show the sub-matrix for debugging	*/
	/*
	printf("Sub-matrix from process %d : \n", proc_idx);
	for (int i = 0; i < blk_num; i++) {
		for (int j = 0; j < col_num; j++) {
			printf("%d ", sub_matrix[i * col_num + j]);
		}
		printf("\n");
	}
	printf("\n");
	*/

	int iteration;
	int display_freq;
	iteration = 3;
	display_freq = 1;

	//show_sub_matrix(sub_matrix);
	//MPI_Barrier(MPI_COMM_WORLD);
	//update_sub_matrix(&sub_matrix);
	//show_sub_matrix(sub_matrix);
	periolically_show(&sub_matrix, iteration, display_freq);

	MPI_Finalize();
	exit(0);
}
