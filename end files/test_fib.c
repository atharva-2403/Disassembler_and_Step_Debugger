#include <stdio.h>

// Recursive Fibonacci function
int fibonacci(int n) {
    if (n <= 0) {
        return 0;
    }
    if (n == 1) {
        return 1;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int main(void) {
    int n = 7;
    int res = fibonacci(n);
    printf("Fibonacci(%d) = %d\n", n, res);
    return 0;
}
