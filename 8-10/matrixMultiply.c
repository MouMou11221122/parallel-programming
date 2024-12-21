#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdint.h>
#include <stdbool.h>

#define BLOCK_LOW(id, p, n)  ((id) * (n) / (p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id) + 1, p, n) - 1)
#define BLOCK_NUMS(id, p, n) (BLOCK_LOW((id) + 1, p, n) - BLOCK_LOW(id, p, n))

int * row_buf;
int * col_buf;
int * submatrix_buf;
int * vec_buf;
int * subvec_buf;
int * result_subvec_partial;
int * result_subvec = NULL;

bool grid_dim_resize (int * grid_dims, int mat_rows, int mat_cols) {
	bool success = false;
	int product = grid_dims[0] * grid_dims[1];
	for (int i = grid_dims[0]; i >= 1; i--) {
		if (product % i == 0 && product / i <= mat_cols) {
			success = true;
			grid_dims[0] = i;
			grid_dims[1] = product / i;	
		}
	}
	return success;
} 



int main (int argc, char* argv[])
{
	int rank, p;
	int grid_dims[2];
	int periodic[2];
	MPI_Comm grid_comm;
	int grid_rows, grid_cols;
	int grid_coords[2];
	int grid_rank;
	MPI_Comm row_comm, col_comm;
	FILE * mat_fp;
	FILE * vec_fp;
	int mat_rows, mat_cols;
	int grid_row_idx, grid_col_idx;

    	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    	MPI_Comm_size(MPI_COMM_WORLD, &p);

	
	mat_fp = fopen(argv[1], "rb");
    	if (mat_fp == NULL) {
        	fprintf(stderr, "Failed to open file : %s\n", argv[1]);
        	MPI_Abort(MPI_COMM_WORLD, 1);
		exit(1);
    	}

	if (fread(&mat_rows, sizeof(int), 1, mat_fp) != 1) {
        	fprintf(stderr, "Failed to read row number\n");
        	fclose(mat_fp);
        	MPI_Abort(MPI_COMM_WORLD, 1);
		exit(1);
    	}

	if (fread(&mat_cols, sizeof(int), 1, mat_fp) != 1) {
        	fprintf(stderr, "Failed to read row number\n");
        	fclose(mat_fp);
        	MPI_Abort(MPI_COMM_WORLD, 1);
		exit(1);
    	}

	grid_dims[0] = grid_dims[1] = 0;
	MPI_Dims_create(p, 2 , grid_dims);
	if (grid_dims[0] > mat_rows) {
		//resize grid dim 1 and grid dim 2
		if(!grid_dim_resize(grid_dims, mat_rows, mat_cols)) {
			fprintf(stderr, "Failed to build the process grid\n");
        		fclose(mat_fp);
        		MPI_Abort(MPI_COMM_WORLD, 1);
			exit(1);
		}
	}
	grid_rows = grid_dims[0];
	grid_cols = grid_dims[1];

	periodic[0] = periodic[1] = 0;
	MPI_Cart_create(MPI_COMM_WORLD, 2, grid_dims, periodic, 1, &grid_comm);

	MPI_Comm_split(grid_comm, grid_coords[0], grid_coords[1], &row_comm);
	MPI_Comm_split(grid_comm, grid_coords[1], grid_coords[0], &col_comm);
	
	//get the coordinate, grid row idx and grid col idx
	MPI_Comm_rank(grid_comm, &grid_rank);
	MPI_Cart_coords(grid_comm, grid_rank, 2, grid_coords);
	grid_row_idx = grid_coords[0]; 
	grid_col_idx = grid_coords[1];

	//build the row comm and col comm	
	MPI_Comm_split(grid_comm, grid_coords[0], grid_coords[1], &row_comm);
	MPI_Comm_split(grid_comm, grid_coords[1], grid_coords[0], &col_comm);

	//allocate submatrix memory
	int submatrix_row_num, submatrix_col_num;
	submatrix_row_num = BLOCK_NUMS(grid_row_idx, grid_rows, mat_rows);
	submatrix_col_num = BLOCK_NUMS(grid_col_idx, grid_cols, mat_cols);
	submatrix_buf = malloc(sizeof(int) * submatrix_row_num * submatrix_col_num);

	//read the matrix.bin
	int rank_in_row;
	int * displs;
	int * send_cnt;
	MPI_Comm_rank(row_comm, &rank_in_row);
	if (!grid_col_idx) {
		//read the partial block of matrix
		displs = malloc(sizeof(int) * grid_cols);
		send_cnt = malloc(sizeof(int) * grid_cols);
		displs[0] = 0;
		for (int i = 0; i < grid_cols; i++) {
			send_cnt[i] = BLOCK_NUMS(i, grid_cols, mat_cols);
			if (i > 0) displs[i] = displs[i - 1] + send_cnt[i - 1];
		}

		//for (int i = 0; i < grid_cols; i++) printf("i : %d, displs :  %d, send_cnt : %d\n", i, displs[i], send_cnt[i]);

		if (fseek(mat_fp, BLOCK_LOW(grid_row_idx, grid_rows, mat_rows) * mat_cols * sizeof(int), SEEK_CUR) != 0) {
			fprintf(stderr, "Failed to move the file offset\n");
        		fclose(mat_fp);
        		exit(1);
    		}
		row_buf = malloc(sizeof(int) * mat_cols);
		for (int i = 0; i < submatrix_row_num; i++) {
			if (fread(row_buf, sizeof(int), mat_cols, mat_fp) < mat_cols) {
    				fprintf(stderr, "Failed to read the matrix.bin\n");
				fclose(mat_fp);
        			MPI_Abort(MPI_COMM_WORLD, 1);
				exit(1);
			}
			MPI_Scatterv(row_buf, send_cnt, displs, MPI_INT, submatrix_buf + submatrix_col_num * i, submatrix_col_num, MPI_INT, 0, row_comm);
		}
		fclose(mat_fp);
		if (grid_row_idx) free(displs);
		if (grid_row_idx) free(send_cnt);
	} else {
		for (int i = 0; i < submatrix_row_num; i++) {
			MPI_Scatterv(NULL, NULL, NULL, MPI_INT, submatrix_buf + submatrix_col_num * i, submatrix_col_num, MPI_INT, 0, row_comm);
			/*if(1) {
				printf("Row [%d] of process (%d,%d) :\n", i, grid_row_idx, grid_col_idx);
				for (int j = 0; j < submatrix_col_num; j++) printf("%d ", *(submatrix_buf + submatrix_col_num * i + j));
				//for (int j = 0; j < submatrix_col_num; j++) printf("%d ", tmp_buf[j]);
				printf("\n");
			}*/
		}
	}

	//read and scatter the vector.bin
	subvec_buf = malloc(sizeof(int) * submatrix_col_num);
	if (grid_row_idx == 0 && grid_col_idx == 0) {
		vec_buf = malloc(sizeof(int) * mat_cols);	
		
		vec_fp = fopen(argv[2], "rb");
    		if (vec_fp == NULL) {
        		fprintf(stderr, "Failed to open file : %s\n", argv[2]);
        		MPI_Abort(MPI_COMM_WORLD, 1);
			exit(1);
    		}

		if (fread(vec_buf, sizeof(int), mat_cols, vec_fp) < mat_cols) {
    			fprintf(stderr, "Failed to read the %s\n", argv[2]);
			fclose(vec_fp);
        		MPI_Abort(MPI_COMM_WORLD, 1);
			exit(1);
		}
		fclose(vec_fp);
		
		MPI_Scatterv(vec_buf, send_cnt, displs, MPI_INT, subvec_buf, submatrix_col_num, MPI_INT, 0, row_comm);
		MPI_Bcast(subvec_buf, submatrix_col_num, MPI_INT, 0, col_comm);
		free(displs);
		free(send_cnt);
	} else if (grid_row_idx == 0) {
		MPI_Scatterv(NULL, NULL, NULL, MPI_INT, subvec_buf, submatrix_col_num, MPI_INT, 0, row_comm);
		MPI_Bcast(subvec_buf, submatrix_col_num, MPI_INT, 0, col_comm);
	} else {
		MPI_Bcast(subvec_buf, submatrix_col_num, MPI_INT, 0, col_comm);
	}
	
	/*
	printf("sub vector from process (%d, %d)\n", grid_row_idx, grid_col_idx);
	for (int i = 0; i < submatrix_col_num; i++) printf("%d ", subvec_buf[i]);
	printf("\n");
	*/

	result_subvec_partial = malloc(sizeof(int) * submatrix_row_num);
	for (int i = 0; i < submatrix_row_num; i++) {
		if (i == 0) result_subvec_partial[i] = 0;
		for (int j = 0; j < submatrix_col_num; j++) {
			result_subvec_partial[i] += subvec_buf[j] * submatrix_buf[i * submatrix_col_num + j];
		}
	}

	if (grid_col_idx == 0) result_subvec = malloc(sizeof(int) * submatrix_row_num);
	MPI_Reduce(result_subvec_partial, result_subvec, submatrix_row_num, MPI_INT, MPI_SUM, 0, row_comm);

	//print the result
	/*if (grid_col_idx == 0) {
		int dummy;
		if (grid_row_idx == 0) {
			for (int i = 0; i < submatrix_row_num; i++) printf("%d\n", result_subvec[i]);
			MPI_Send(&dummy, 1, MPI_INT, grid_row_idx + 1, 1, col_comm);
		} else if (grid_row_idx == grid_rows - 1) {
			MPI_Status status;
			MPI_Recv(&dummy, 1, MPI_INT, grid_row_idx - 1, 1, col_comm, &status);	
			for (int i = 0; i < submatrix_row_num; i++) printf("%d\n", result_subvec[i]);
		} else {
			MPI_Status status;
			MPI_Recv(&dummy, 1, MPI_INT, grid_row_idx - 1, 1, col_comm, &status);	
			for (int i = 0; i < submatrix_row_num; i++) printf("%d\n", result_subvec[i]);
			MPI_Send(&dummy, 1, MPI_INT, grid_row_idx + 1, 1, col_comm);
		}
	}*/
	
	//print the result
	if (grid_col_idx == 0) {
		if (grid_row_idx == 0) {
			int * displs;
			int * recv_cnt;
			col_buf = malloc(sizeof(int) * mat_rows);
			displs = malloc(sizeof(int) * grid_rows);
			recv_cnt = malloc(sizeof(int) * grid_rows);
			displs[0] = 0;
			for (int i = 0; i < grid_rows; i++) {
				recv_cnt[i] = BLOCK_NUMS(i, grid_rows, mat_rows);
				if (i > 0) displs[i] = displs[i - 1] + recv_cnt[i - 1];
			}
			MPI_Gatherv(result_subvec, submatrix_row_num, MPI_INT, col_buf, recv_cnt, displs, MPI_INT, 0, col_comm);
			for (int i = 0; i < mat_rows; i++) printf("%d\n", col_buf[i]);
			free(displs);
			free(recv_cnt);
			free(col_buf);
		} else {
			MPI_Gatherv(result_subvec, submatrix_row_num, MPI_INT, NULL, NULL, NULL, MPI_INT, 0, col_comm);
		}
	}	

	/*	
	if (grid_col_idx == 0) {
		for (int i = 0; i < submatrix_row_num; i++) printf("%d\n", result_subvec[i]);
	}
	*/

	/*	utilities
	MPI_Comm_rank(grid_comm, &grid_rank);
	MPI_Cart_coords(grid_comm, grid_rank, 2, grid_coords);
	printf("process rank %d, gird rank %d,coordinates : (%d, %d)\n", rank, grid_rank, grid_coords[0], grid_coords[1]);
	*/

	MPI_Finalize();
	exit(0);
}
