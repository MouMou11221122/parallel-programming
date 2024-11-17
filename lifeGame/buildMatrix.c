#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main() {
    int rows = 5;
    int cols = 5;

    FILE *file = fopen("matrix.bin", "wb");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }

    fwrite(&rows, sizeof(int), 1, file);
    fwrite(&cols, sizeof(int), 1, file);

	/*
	bool value = true;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            fwrite(&value, sizeof(bool), 1, file);
			value = !value;
        }
    }
	*/

	bool value;

	value = true;
	fwrite(&value, sizeof(bool), 1, file);
	value = false;
	fwrite(&value, sizeof(bool), 1, file);
	value = false;
	fwrite(&value, sizeof(bool), 1, file);
	value = true;
	fwrite(&value, sizeof(bool), 1, file);
	value = true;
	fwrite(&value, sizeof(bool), 1, file);


	value = false;
	fwrite(&value, sizeof(bool), 1, file);
	value = false;
	fwrite(&value, sizeof(bool), 1, file);
	value = false;
	fwrite(&value, sizeof(bool), 1, file);
	value = true;
	fwrite(&value, sizeof(bool), 1, file);
	value = true;
	fwrite(&value, sizeof(bool), 1, file);


	value = false;
	fwrite(&value, sizeof(bool), 1, file);
	value = false;
	fwrite(&value, sizeof(bool), 1, file);
	value = false;
	fwrite(&value, sizeof(bool), 1, file);
	value = false;
	fwrite(&value, sizeof(bool), 1, file);
	value = true;
	fwrite(&value, sizeof(bool), 1, file);

	value = true;
	fwrite(&value, sizeof(bool), 1, file);
	value = true;
	fwrite(&value, sizeof(bool), 1, file);
	value = true;
	fwrite(&value, sizeof(bool), 1, file);
	value = false;
	fwrite(&value, sizeof(bool), 1, file);
	value = true;
	fwrite(&value, sizeof(bool), 1, file);

	value = false;
	fwrite(&value, sizeof(bool), 1, file);
	value = false;
	fwrite(&value, sizeof(bool), 1, file);
	value = false;
	fwrite(&value, sizeof(bool), 1, file);
	value = false;
	fwrite(&value, sizeof(bool), 1, file);
	value = true;
	fwrite(&value, sizeof(bool), 1, file);


    fclose(file);

    printf("Matrix written to matrix.bin\n");
    return 0;
}

