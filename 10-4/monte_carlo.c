#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <time.h>
#include "utilities.h"

double distance_to_diagonal_full(double x, double y, double z) {
    double dot_product = x + y + z;  
    double magnitude = sqrt(x * x + y * y + z * z);  
    double angle_cos = dot_product / (sqrt(3) * magnitude);  
    double angle_sin = sqrt(1 - angle_cos * angle_cos);  
    return angle_sin * magnitude;  
}

int monte_carlo_count_inside(double cube_side, double cylinder_diameter, int num_samples) {
    double cylinder_radius = cylinder_diameter / 2.0;  
    int points_inside = 0;

    for (int i = 0; i < num_samples; i++) {
        double x = ((double)rand() / RAND_MAX) * cube_side;
        double y = ((double)rand() / RAND_MAX) * cube_side;
        double z = ((double)rand() / RAND_MAX) * cube_side;

        double distance = distance_to_diagonal_full(x, y, z);

        if (distance <= cylinder_radius) points_inside++;
    }

    return points_inside;
}

int main(int argc, char** argv) {
	double start_time, end_time;
    int num_procs, rank;
    double cube_side = 2.0;  
    double cylinder_diameter = 0.3;  
    int total_samples = 1000000000;  
    int local_samples, local_inside, global_inside;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    local_samples = total_samples / num_procs;

    srand(time(NULL) + rank);
	start_time = MPI_Wtime();
    local_inside = monte_carlo_count_inside(cube_side, cylinder_diameter, local_samples);

    MPI_Reduce(&local_inside, &global_inside, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        double cube_volume = pow(cube_side, 3);  
        double cylinder_volume_ratio = (double)global_inside / total_samples;  
        double remaining_volume = cube_volume * (1 - cylinder_volume_ratio);  
		end_time = MPI_Wtime();
		output_real_exec_time(end_time - start_time);
        printf("remain volumn: %.5f\n", remaining_volume);
    }

    MPI_Finalize();
    return 0;
}

