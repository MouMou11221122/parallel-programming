#include <stdio.h>
#include <math.h>

int is_prime(int num) {
    if (num <= 1) {
        return 0;  
    }
    if (num == 2) {
        return 1;  
    }
    if (num % 2 == 0) {
        return 0;  
    }
    
    for (int i = 3; i <= sqrt(num); i += 2) {
        if (num % i == 0) {
            return 0;
        }
    }
    return 1;  
}

void count_consecutive_odd_primes(int n) {
    int count = 0;
    
    for (int i = 3; i <= n - 2; i += 2) {
        if (is_prime(i) && is_prime(i + 2)) {
            count++;
            //printf("Consecutive odd primes: (%d, %d)\n", i, i + 2);
        }
    }
    
    printf("Total pairs of consecutive odd primes less than %d: %d\n", n, count);
}

int main() {
    int n;
    
    printf("Enter a number n: ");
    scanf("%d", &n);
    
    count_consecutive_odd_primes(n);
    
    return 0;
}

