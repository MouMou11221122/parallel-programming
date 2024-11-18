#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <limits.h>

typedef int dtype;
#define MPI_TYPE MPI_INT 
#define PTR_SIZE sizeof(long)


#define DATA_MSG			 	 0
#define PROMPT_MSG				 1
#define RESPONSE_MSG				 2
#define OPEN_FILE_ERROR				-1

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define BLOCK_LOW(id, p, n)  ((id) * (n) / (p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id) + 1, p, n) - 1)
#define BLOCK_SIZE(id, p, n) (BLOCK_LOW((id) + 1, p, n) - BLOCK_LOW(id, p, n))
#define BLOCK_OWNER(index, p, n) (((p) * ((index) + 1) - 1) / (n))



void print_submatrix(
    void **a,               /* OUT - Doubly subscripted array */
    MPI_Datatype dtype,     /* OUT - Type of array elements */
    int rows,               /* OUT - Matrix rows */
    int cols)               /* OUT - Matrix cols */
{
    int i, j;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            if (dtype == MPI_DOUBLE)
                printf("%6.3f ", ((double **)a)[i][j]);
            else {
                if (dtype == MPI_FLOAT)
                    printf("%6.3f ", ((float **)a)[i][j]);
                else if (dtype == MPI_INT)
                    printf("%6d ", ((int **)a)[i][j]);
            }
        }
        putchar('\n');
    }
}



void read_row_striped_matrix (
	char				*s,				/* IN - File name */
	void				***subs,			/* OUT - 2D submatrix indices */
	void				**storage,			/* OUT - Submatrix stored here */
	MPI_Datatype			dtype,				/* IN - Matrix element type */
	int				*m,				/* OUT - Matrix rows */
	int				*n,				/* OUT - Matrix rows */
	MPI_Comm			comm				/* IN - Communicator */
) 
{
	int			datum_size;
	int 		id;
	FILE		*infileptr;
	int			local_rows;
	void		**lptr;
	int			p;
	void		*rptr;
	MPI_Status	status;
	int			x;
	
	MPI_Comm_size(comm, &p);
	MPI_Comm_rank(comm, &id);
	datum_size = sizeof(dtype);

	if (id == p - 1) {
		infileptr = fopen(s, "rb");
		if (infileptr == NULL) *m = 0;
		else {
			fread(m, sizeof(int), 1, infileptr);
			fread(n, sizeof(int), 1, infileptr);
		}
	}
	MPI_Bcast(m, 1, MPI_INT, p - 1, comm);

	if (!*m) MPI_Abort(MPI_COMM_WORLD, OPEN_FILE_ERROR); 

	MPI_Bcast(n, 1, MPI_INT, p - 1, comm);

	local_rows = BLOCK_SIZE(id, p, *m);

	*storage = (void *) malloc(local_rows * *n * datum_size);
	*subs = (void **) malloc(local_rows * PTR_SIZE);
	
	lptr = (void **) *subs;
	rptr = (void *) *storage;
	for (int i = 0; i < local_rows; i++) {
		*(lptr++) = (void *)rptr;
		rptr += *n * datum_size;
	}

	if (id == p - 1) {
		for(int i = 0; i < p - 1; i++) {
			x = fread(*storage, datum_size, BLOCK_SIZE(i, p, *m) * *n, infileptr);
			//printf("Send to process %d %d rows and %d elements and n : %d\n", i, BLOCK_SIZE(i, p, *m), x, *n);
			for (int k = 0; k < BLOCK_SIZE(i, p, *m); k++) {
				for (int j = 0; j < *n; j++) {
					printf("%d ", *((int *)(*storage) + k * *n + j));
				}
				printf("\n");
			}
			MPI_Send(*storage, BLOCK_SIZE(i, p, *m) * *n, dtype, i, DATA_MSG, comm);
		}
		x = fread(*storage, datum_size, local_rows * *n, infileptr);
		fclose(infileptr);
	} else {
		MPI_Recv(*storage, local_rows * *n, dtype, p - 1, DATA_MSG, comm, &status);
	}
}

void print_row_striped_matrix (
	void			**a,
	MPI_Datatype	dtype,
	int				m,
	int				n,
	MPI_Comm 		comm) 
{
	MPI_Status	status;				/* Result of receive */
	void		*bstorage;			/* Elements received from another process */
	void		**b;				/* 2D array indexing into bstorage */
	int			datum_size;		/* Bytes per element */
	int			id;			/* Process rank */
	int			local_rows;		/* This proc's rows */
	int			max_block_size;		/* Most matrix rows held by any process */
	int			prompt;			/* Dummy variable */
	int			p;			/* Number of processes */
	int			i;	

	MPI_Comm_rank(comm, &id);
	MPI_Comm_size(comm, &p);
	local_rows = BLOCK_SIZE(id, p, m);
	if (!id) {
		print_submatrix(a, dtype, local_rows, n);
		if (p > 1) {
			datum_size = sizeof(dtype);
			max_block_size = BLOCK_SIZE(p - 1, p, m);
			bstorage = (void *) malloc(max_block_size * n * datum_size);
			b = (void **) malloc(max_block_size * PTR_SIZE);
			b[0] =  bstorage;
			for (i = 1; i < max_block_size; i++) {
    			b[i] = b[i-1] + n * datum_size;
			}
			for (i = 1; i < p; i++) {
    			MPI_Send (&prompt, 1, MPI_INT, i, PROMPT_MSG,
              	MPI_COMM_WORLD);
   				MPI_Recv (bstorage, BLOCK_SIZE(i,p,m)*n, dtype,
              	i, RESPONSE_MSG, MPI_COMM_WORLD, &status);
    			print_submatrix (b, dtype, BLOCK_SIZE(i,p,m), n);
			}
			free (b);
			free (bstorage);
		}
		putchar('\n');
	} else {
		MPI_Recv(&prompt, 1, MPI_INT, 0, PROMPT_MSG, MPI_COMM_WORLD, &status);
		MPI_Send(*a, local_rows * n, dtype, 0, RESPONSE_MSG, MPI_COMM_WORLD);
	}
}

void compute_shortest_paths (int id, int p, dtype **a, int n)
{
    int i, j, k;
    int offset;        /* Local index of broadcast row */
    int root;          /* Process controlling row to be bcast */
    int* tmp;          /* Holds the broadcast row */

    tmp = (dtype *) malloc (n * sizeof(dtype));
    for (k = 0; k < n; k++) {
        root = BLOCK_OWNER(k,p,n);
        if (root == id) {
            offset = k - BLOCK_LOW(id,p,n);
            for (j = 0; j < n; j++)
                tmp[j] = a[offset][j];
        }
        MPI_Bcast (tmp, n, MPI_TYPE, root, MPI_COMM_WORLD);
        for (i = 0; i < BLOCK_SIZE(id,p,n); i++)
            for (j = 0; j < n; j++) {
		if (a[i][k] == INT_MAX || tmp[j] == INT_MAX) continue; 
                a[i][j] = MIN(a[i][j],a[i][k]+tmp[j]);
	    }
    }
    free (tmp);
}


int main (int argc, char *argv[]) {
    dtype** a;          /* Doubly-subscripted array */
    dtype* storage;     /* Local portion of array elements */
    int i, j, k;
    int id;             /* Process rank */
    int m;              /* Rows in matrix */
    int n;              /* Columns in matrix */
    int p;              /* Number of processes */

    void compute_shortest_paths (int, int, int**, int);

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &id);
    MPI_Comm_size (MPI_COMM_WORLD, &p);

    read_row_striped_matrix (argv[1], (void *) &a,
        (void *) &storage, MPI_TYPE, &m, &n, MPI_COMM_WORLD);

    if (m != n) { 
	fprintf (stderr, "Matrix must be square\n");
	exit(1);
    }
    print_row_striped_matrix ((void **) a, MPI_TYPE, m, n,
        MPI_COMM_WORLD);
    compute_shortest_paths (id, p, (dtype **) a, n);
    print_row_striped_matrix ((void **) a, MPI_TYPE, m, n,
        MPI_COMM_WORLD);
    MPI_Finalize();
	exit(0);
}




