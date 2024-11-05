#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void sieve(bool *is_prime, int n) {
    // Initialize all entries as prime
    for (int i = 2; i < n; i++) {
        is_prime[i] = true;
    }

    // Apply Sieve of Eratosthenes
    for (int p = 2; p * p < n; p++) {
        if (is_prime[p]) {
            for (int multiple = p * p; multiple < n; multiple += p) {
                is_prime[multiple] = false;
            }
        }
    }
}

int main() {
    int n;
    printf("Enter an integer n: ");
    scanf("%d", &n);

    if (n < 3) {
        printf("No consecutive primes found for n < 3.\n");
        return 0;
    }

    // Array to mark prime numbers
    bool *is_prime = (bool *)malloc(n * sizeof(bool));
    sieve(is_prime, n);

    // Variables to keep track of the largest gap and the corresponding primes
    int previous_prime = 2;  // First prime number
    int max_gap = 0;
    int prime1 = 0, prime2 = 0;

    for (int i = 3; i < n; i++) {
        if (is_prime[i]) {
            int gap = i - previous_prime;
            if (gap > max_gap) {
                max_gap = gap;
                prime1 = previous_prime;
                prime2 = i;
            }
            previous_prime = i;
        }
    }

    if (max_gap > 0) {
        printf("The largest gap between consecutive primes less than %d is %d.\n", n, max_gap);
        //printf("The two consecutive primes with the largest gap are %d and %d.\n", prime1, prime2);
    } else {
        printf("No consecutive primes found.\n");
    }

    free(is_prime);
    return 0;
}

