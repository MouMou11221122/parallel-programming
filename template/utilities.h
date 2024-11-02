#include <stdio.h>
#include <stdlib.h>

void output_real_exec_time (double retime) {
	FILE *fp = fopen("time.log", "a");
    if (fp == NULL) {
        perror("Failed to open file");
        exit(1);
    }
	fprintf(fp, "%lf\n", retime);
	fclose(fp);
}
