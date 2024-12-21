#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdint.h>

#define BLOCK_LOW(id, p, n)  ((id) * (n) / (p))


int main (int argc, char* argv[])
{
	int rank, p;
	int dims[2];
	int periodic[2];
	MPI_Comm grid_comm;
	int grid_rows, grid_cols;
	int grid_coords[2];
	int grid_rank;
	MPI_Comm row_comm, col_comm;
	FILE * fp;
	int mat_rows, mat_cols;

    MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

	file = fopen(argv[1], "rb");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file : %s\n", argv[1]);
        MPI_Abort();
		exit(1);
    }

	if (fread(&mat_rows, sizeof(uint32_t), 1, file) != 1) {
        fprintf(stderr, "Failed to read row number\n");
        fclose(file);
        MPI_Abort();
		exit(1);
    }

	if (fread(&mat_cols, sizeof(uint32_t), 1, file) != 1) {
        fprintf(stderr, "Failed to read row number\n");
        fclose(file);
        MPI_Abort();
		exit(1);
    }

	dims[0] = dims[1] = 0;
	MPI_Dims_create(p, 2 , dims);
	if (dim[0] > mat_rows) {
		dims[0] = 
	}
	grid_rows = dims[0];
	grid_cols = dims[1];

	periodic[0] = periodic[1] = 0;
	MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periodic, 1, &grid_comm);

	MPI_Comm_rank(grid_comm, &grid_rank);
	MPI_Cart_coords(grid_comm, grid_rank, 2, grid_coords);
	//printf("process rank %d, gird rank %d,coordinates : (%d, %d)\n", rank, grid_rank, grid_coords[0], grid_coords[1]);
	
	MPI_Comm_Split(grid_comm, grid_coords[0], grid_coords[1], &row_comm);
	MPI_Comm_Split(grid_comm, grid_coords[1], grid_coords[0], &col_comm);

	MPI_Finalize();
	exit(0);
}
