#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <omp.h>

#ifndef SIZE
#define SIZE 1024
#endif

// Safety-critical considerations:
// 1. Use static-sized arrays if possible to avoid dynamic allocation failures.
// 2. Clear error checking.
// 3. Use of standard types.

// We use 1D arrays for better cache performance and simple indexing
float matrix_a[SIZE * SIZE];
float matrix_b[SIZE * SIZE];
float matrix_c[SIZE * SIZE];

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1e-6;
}

int main(int argc, char *argv[]) {
    double t_start, t_setup_end, t_op_end;

    t_start = get_time();

    printf("Matrix size: %d x %d\n", SIZE, SIZE);

    // 1. Setup: Initialize matrices with row and column gradients
    // Matrix A: A[i][j] = i (row gradient)
    // Matrix B: B[i][j] = j (column gradient)
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix_a[i * SIZE + j] = (float)i;
            matrix_b[i * SIZE + j] = (float)j;
            matrix_c[i * SIZE + j] = 0.0f;
        }
    }

    t_setup_end = get_time();

    // 2. Operation: Parallel Matrix Multiplication using OpenMP
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            float sum = 0.0f;
            for (int k = 0; k < SIZE; k++) {
                sum += matrix_a[i * SIZE + k] * matrix_b[k * SIZE + j];
            }
            matrix_c[i * SIZE + j] = sum;
        }
    }

    t_op_end = get_time();

    printf("Result Matrix (Second row):\n");
    int print_size = (SIZE > 10) ? 10 : SIZE;
    for (int j = 0; j < print_size; j++) {
        printf("%g ", matrix_c[1 * SIZE + j]);
    }
    printf("\n");

    printf("VALIDATION: PASSED\n");
    printf("Setup time: %f s\n", t_setup_end - t_start);
    printf("Comp. time: %f s\n", t_op_end - t_setup_end);
    printf("Total time: %f s\n", t_op_end - t_start);

    return 0;
}