#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "matrix_sum.h"

static void fill_matrix_random(float* mat, size_t M, size_t N) {
    size_t total = M * N;
    for (size_t i = 0; i < total; i++) {
        mat[i] = (float)(rand() % 100) / 10.0f;
    }
}

static int verify_result(const float** matrices, const float* result, size_t M, size_t N) {
    size_t total = M * N;
    float* expected = (float*)calloc(total, sizeof(float));
    
    for (size_t k = 0; k < NUM_MATRICES; k++) {
        for (size_t i = 0; i < total; i++) {
            expected[i] += matrices[k][i];
        }
    }
    
    const float epsilon = 1e-5f;
    int passed = 1;
    size_t errors = 0;
    
    for (size_t i = 0; i < total; i++) {
        if (result[i] < expected[i] - epsilon || result[i] > expected[i] + epsilon) {
            if (errors < 5) {
                printf("Error at index %zu: expected %.6f, got %.6f\n", 
                       i, expected[i], result[i]);
            }
            errors++;
            passed = 0;
        }
    }
    
    if (!passed) {
        printf("Total errors: %zu\n", errors);
    }
    
    free(expected);
    return passed;
}

int main(int argc, char* argv[]) {
    size_t M = 256;
    size_t N = 256;
    
    if (argc >= 3) {
        M = (size_t)atoi(argv[1]);
        N = (size_t)atoi(argv[2]);
    }
    
    printf("Matrix size: %zux%zu\n", M, N);
    printf("Number of matrices: %d\n", NUM_MATRICES);
    
    float** matrices = (float**)malloc(NUM_MATRICES * sizeof(float*));
    for (int i = 0; i < NUM_MATRICES; i++) {
        matrices[i] = (float*)malloc(M * N * sizeof(float));
    }
    float* result = (float*)malloc(M * N * sizeof(float));
    
    srand((unsigned int)time(NULL));
    for (int i = 0; i < NUM_MATRICES; i++) {
        fill_matrix_random(matrices[i], M, N);
    }
    
    const float** mat_ptrs = (const float**)matrices;
    
    clock_t start = clock();
    matrix_sum_single_thread(mat_ptrs, result, M, N);
    clock_t end = clock();
    
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Single-thread time: %.4f seconds\n", elapsed);
    
    printf("Verifying result...\n");
    int passed = verify_result(mat_ptrs, result, M, N);
    
    if (passed) {
        printf("Test PASSED\n");
    } else {
        printf("Test FAILED\n");
    }
    
    for (int i = 0; i < NUM_MATRICES; i++) {
        free(matrices[i]);
    }
    free(matrices);
    free(result);
    
    return passed ? 0 : 1;
}