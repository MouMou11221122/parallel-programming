#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    const char *matrix_filename = "matrix.bin"; // 矩陣檔案名稱
    const char *vector_filename = "vector.bin"; // 向量檔案名稱
    int m, n;

    // 請使用者輸入矩陣的維度
    printf("請輸入矩陣的行數 (m): ");
    if (scanf("%d", &m) != 1 || m <= 0) {
        fprintf(stderr, "行數輸入無效，必須為正整數。\n");
        return EXIT_FAILURE;
    }

    printf("請輸入矩陣的列數 (n): ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "列數輸入無效，必須為正整數。\n");
        return EXIT_FAILURE;
    }

    // 開啟 matrix.bin 進行二進位寫入
    FILE *matrix_file = fopen(matrix_filename, "wb");
    if (!matrix_file) {
        perror("無法開啟矩陣檔案");
        return EXIT_FAILURE;
    }

    // 寫入矩陣維度 m 和 n
    if (fwrite(&m, sizeof(int), 1, matrix_file) != 1 || fwrite(&n, sizeof(int), 1, matrix_file) != 1) {
        perror("寫入矩陣維度失敗");
        fclose(matrix_file);
        return EXIT_FAILURE;
    }

    // 設定隨機數種子
    srand((unsigned)time(NULL));

    // 產生矩陣並寫入到 matrix.bin
    int **matrix = (int **)malloc(m * sizeof(int *));
    if (!matrix) {
        fprintf(stderr, "記憶體分配失敗。\n");
        fclose(matrix_file);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < m; i++) {
        matrix[i] = (int *)malloc(n * sizeof(int));
        if (!matrix[i]) {
            fprintf(stderr, "記憶體分配失敗。\n");
            for (int k = 0; k < i; k++) free(matrix[k]);
            free(matrix);
            fclose(matrix_file);
            return EXIT_FAILURE;
        }

        for (int j = 0; j < n; j++) {
            matrix[i][j] = rand() % 100; // 產生小於 100 的隨機整數
            if (fwrite(&matrix[i][j], sizeof(int), 1, matrix_file) != 1) {
                perror("寫入矩陣元素失敗");
                for (int k = 0; k <= i; k++) free(matrix[k]);
                free(matrix);
                fclose(matrix_file);
                return EXIT_FAILURE;
            }
        }
    }

    fclose(matrix_file); // 關閉 matrix.bin 檔案

    // 打印矩陣
    printf("生成的矩陣為：\n");
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

    // 開啟 vector.bin 進行二進位寫入
    FILE *vector_file = fopen(vector_filename, "wb");
    if (!vector_file) {
        perror("無法開啟向量檔案");
        return EXIT_FAILURE;
    }

    // 產生向量並寫入到 vector.bin
    int *vector = (int *)malloc(n * sizeof(int));
    if (!vector) {
        fprintf(stderr, "記憶體分配失敗。\n");
        fclose(vector_file);
        return EXIT_FAILURE;
    }

    printf("生成的向量為：\n");
    for (int i = 0; i < n; i++) {
        vector[i] = rand() % 1000; // 產生小於 1000 的隨機整數
        if (fwrite(&vector[i], sizeof(int), 1, vector_file) != 1) {
            perror("寫入向量元素失敗");
            free(vector);
            fclose(vector_file);
            return EXIT_FAILURE;
        }
        printf("%d\t", vector[i]); // 打印向量
    }
    printf("\n");

    fclose(vector_file); // 關閉 vector.bin 檔案

    // 釋放記憶體
    free(vector);

    printf("矩陣已成功寫入 %s\n", matrix_filename);
    printf("向量已成功寫入 %s\n", vector_filename);
    return EXIT_SUCCESS;
}

