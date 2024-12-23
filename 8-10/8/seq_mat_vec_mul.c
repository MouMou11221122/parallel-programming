#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "使用方法: %s <matrix.bin> <vector.bin>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *matrix_file = argv[1];
    const char *vector_file = argv[2];

    // 開啟 matrix.bin
    FILE *matrix_fp = fopen(matrix_file, "rb");
    if (!matrix_fp) {
        perror("無法開啟 matrix.bin");
        return EXIT_FAILURE;
    }

    // 讀取矩陣的維度
    int m, n;
    if (fread(&m, sizeof(int), 1, matrix_fp) != 1 || fread(&n, sizeof(int), 1, matrix_fp) != 1) {
        fprintf(stderr, "讀取矩陣維度失敗。\n");
        fclose(matrix_fp);
        return EXIT_FAILURE;
    }

    // 開啟 vector.bin
    FILE *vector_fp = fopen(vector_file, "rb");
    if (!vector_fp) {
        perror("無法開啟 vector.bin");
        fclose(matrix_fp);
        return EXIT_FAILURE;
    }

    // 確認 vector 的大小與矩陣列數匹配
    fseek(vector_fp, 0, SEEK_END);
    long vector_size = ftell(vector_fp);
    fseek(vector_fp, 0, SEEK_SET);

    if (vector_size != n * sizeof(int)) {
        fprintf(stderr, "向量大小與矩陣列數不匹配。\n");
        fclose(matrix_fp);
        fclose(vector_fp);
        return EXIT_FAILURE;
    }

    // 讀取 vector
    int *vector = (int *)malloc(n * sizeof(int));
    if (!vector) {
        fprintf(stderr, "記憶體分配失敗。\n");
        fclose(matrix_fp);
        fclose(vector_fp);
        return EXIT_FAILURE;
    }
    if (fread(vector, sizeof(int), n, vector_fp) != n) {
        fprintf(stderr, "讀取向量失敗。\n");
        free(vector);
        fclose(matrix_fp);
        fclose(vector_fp);
        return EXIT_FAILURE;
    }
    fclose(vector_fp);

    // 計算矩陣-向量乘積
    int *result = (int *)malloc(m * sizeof(int));
    if (!result) {
        fprintf(stderr, "記憶體分配失敗。\n");
        free(vector);
        fclose(matrix_fp);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < m; i++) {
        result[i] = 0; // 初始化結果
        for (int j = 0; j < n; j++) {
            int element;
            if (fread(&element, sizeof(int), 1, matrix_fp) != 1) {
                fprintf(stderr, "讀取矩陣元素失敗。\n");
                free(vector);
                free(result);
                fclose(matrix_fp);
                return EXIT_FAILURE;
            }
            result[i] += element * vector[j];
        }
    }
    fclose(matrix_fp);

    // 打印結果向量
    for (int i = 0; i < m; i++) {
        printf("%d\n", result[i]);
    }

    // 釋放記憶體
    free(vector);
    free(result);

    return EXIT_SUCCESS;
}

