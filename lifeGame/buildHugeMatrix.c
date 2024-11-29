#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <m> <n> <output_file>\n", argv[0]);
        return 1;
    }

    int m = atoi(argv[1]);
    int n = atoi(argv[2]);
    char *output_filename = argv[3];

    if (m <= 0 || n <= 0) {
        fprintf(stderr, "Error: m and n must be positive integers.\n");
        return 1;
    }

    FILE *output_file = fopen(output_filename, "wb");
    if (output_file == NULL) {
        perror("Error opening file");
        return 1;
    }

    srand(time(NULL));

    fwrite(&m, sizeof(int), 1, output_file);
    fwrite(&n, sizeof(int), 1, output_file);

    for (int i = 0; i < m * n; i++) {
        unsigned char element = rand() % 2;  
        fwrite(&element, sizeof(unsigned char), 1, output_file);
    }

    fclose(output_file);

    printf("Binary file '%s' has been created with a %d x %d matrix.\n", output_filename, m, n);

    exit(0);
}

