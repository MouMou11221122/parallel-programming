#include <stdio.h>
#include <stdlib.h>

int main() {
    const char *filename = "vector.bin"; // 向量檔案名稱

    // 開啟檔案以進行二進位讀取
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("無法開啟向量檔案");
        return EXIT_FAILURE;
    }

    // 獲取檔案大小，確定向量元素的數量
    fseek(file, 0, SEEK_END); // 將檔案指標移到檔案末尾
    long file_size = ftell(file); // 取得檔案大小
    rewind(file); // 重置檔案指標回到檔案起始位置

    if (file_size % sizeof(int) != 0) {
        fprintf(stderr, "檔案大小不符合向量結構，可能已損壞。\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    int element_count = file_size / sizeof(int); // 計算向量元素數量

    // 分配記憶體來存儲向量元素
    int *vector = (int *)malloc(element_count * sizeof(int));
    if (!vector) {
        fprintf(stderr, "記憶體分配失敗。\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    // 讀取向量元素
    if (fread(vector, sizeof(int), element_count, file) != (size_t)element_count) {
        perror("讀取向量元素失敗");
        free(vector);
        fclose(file);
        return EXIT_FAILURE;
    }

    fclose(file); // 關閉檔案

    // 打印向量內容
    printf("向量內容為：\n");
    for (int i = 0; i < element_count; i++) {
        printf("%d ", vector[i]);
    }
    printf("\n");

    // 釋放記憶體
    free(vector);

    return EXIT_SUCCESS;
}

