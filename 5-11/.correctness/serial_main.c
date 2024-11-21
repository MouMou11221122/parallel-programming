#include <stdio.h>
#include <stdlib.h>
#define foo(n) ((n) * (n + 1) / 2)

int main (int argc, char** argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s [n]\n", argv[0]);
		exit(1);
	}
	printf("result : %lld\n", foo(atoll(argv[1])));
	exit(0);
}
