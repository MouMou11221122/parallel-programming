#include <stdio.h>

int main() {
    FILE *file = fopen("matrix.bin", "wb");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    int numbers[] = {
        6, 6, 0, 2, 5, 
        2147483647, 2147483647, 2147483647, 2147483647, 0, 
        7, 1, 2147483647, 8, 2147483647, 
        2147483647, 0, 4, 2147483647, 2147483647, 
        2147483647, 2147483647, 2147483647, 0, 3, 
        2147483647, 2147483647, 2147483647, 2, 2147483647, 
        0, 3, 2147483647, 5, 2147483647, 2, 4, 0
    };
    
    size_t numElements = sizeof(numbers) / sizeof(numbers[0]);  

    size_t elementsWritten = fwrite(numbers, sizeof(int), numElements, file);

    if (elementsWritten != numElements) {
        printf("Error: Unable to write the correct number of elements.\n");
    } else {
        printf("Successfully wrote %zu elements to the file.\n", elementsWritten);
    }

    fclose(file);
    return 0;
}

