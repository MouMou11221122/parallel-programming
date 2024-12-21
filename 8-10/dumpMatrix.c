#include <stdio.h>
#include <stdlib.h>

int main() {
    const char *filename = "matrix.bin"; // 要讀取的檔案名稱
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("無法開啟檔案");
        return EXIT_FAILURE;
    }

    // 讀取 m 和 n
    int m, n;
    if (fread(&m, sizeof(int), 1, file) != 1 || fread(&n, sizeof(int), 1, file) != 1) {
        perror("讀取矩陣維度失敗");
        fclose(file);
        return EXIT_FAILURE;
    }

    // 動態分配記憶體存放矩陣
    int **matrix = (int **)malloc(m * sizeof(int *));
    if (!matrix) {
        fprintf(stderr, "記憶體分配失敗。\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < m; i++) {
        matrix[i] = (int *)malloc(n * sizeof(int));
        if (!matrix[i]) {
            fprintf(stderr, "記憶體分配失敗。\n");
            for (int k = 0; k < i; k++) free(matrix[k]);
            free(matrix);
            fclose(file);
            return EXIT_FAILURE;
        }
    }

    // 讀取矩陣內容
    for (int i = 0; i < m; i++) {
        if (fread(matrix[i], sizeof(int), n, file) != (size_t)n) {
            perror("讀取矩陣元素失敗");
            for (int k = 0; k < m; k++) free(matrix[k]);
            free(matrix);
            fclose(file);
            return EXIT_FAILURE;
        }
    }

    fclose(file);

    // 打印矩陣
    printf("從檔案 '%s' 中讀取的矩陣內容如下：\n", filename);
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d\t", matrix[i][j]);
        }
        printf("\n");
    }

    // 釋放記憶體
    for (int i = 0; i < m; i++) {
        free(matrix[i]);
    }
    free(matrix);

    return EXIT_SUCCESS;
}

