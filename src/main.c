#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "matrix_sum.h"

static double get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

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

static int compare_results(const float* a, const float* b, size_t M, size_t N) {
    size_t total = M * N;
    const float epsilon = 1e-3f;
    int passed = 1;
    size_t errors = 0;
    
    for (size_t i = 0; i < total; i++) {
        float diff = a[i] - b[i];
        if (diff < 0) diff = -diff;
        
        if (diff > epsilon) {
            if (errors < 5) {
                printf("Mismatch at index %zu: single=%.6f, multi=%.6f, diff=%.6f\n", 
                       i, a[i], b[i], diff);
            }
            errors++;
            passed = 0;
        }
    }
    
    if (!passed) {
        printf("Total mismatches: %zu\n", errors);
    }
    
    return passed;
}

int main(int argc, char* argv[]) {
    size_t M = 256;
    size_t N = 256;
    int num_threads = 6;
    
    if (argc >= 3) {
        M = (size_t)atoi(argv[1]);
        N = (size_t)atoi(argv[2]);
    }
    if (argc >= 4) {
        num_threads = atoi(argv[3]);
    }
    
    printf("Matrix size: %zux%zu\n", M, N);
    printf("Number of matrices: %d\n", NUM_MATRICES);
    printf("Number of threads: %d\n\n", num_threads);
    
    float** matrices = (float**)malloc(NUM_MATRICES * sizeof(float*));
    for (int i = 0; i < NUM_MATRICES; i++) {
        matrices[i] = (float*)malloc(M * N * sizeof(float));
    }
    float* result_single = (float*)malloc(M * N * sizeof(float));
    float* result_multi = (float*)malloc(M * N * sizeof(float));
    
    srand((unsigned int)time(NULL));
    for (int i = 0; i < NUM_MATRICES; i++) {
        fill_matrix_random(matrices[i], M, N);
    }
    
    const float** mat_ptrs = (const float**)matrices;
    
    double start = get_time_ns();
    matrix_sum_single_thread(mat_ptrs, result_single, M, N);
    double end = get_time_ns();
    double time_single = end - start;
    
    printf("Single-thread time: %.6f seconds\n", time_single);
    
    start = get_time_ns();
    matrix_sum_multi_thread(mat_ptrs, result_multi, M, N, num_threads);
    end = get_time_ns();
    double time_multi = end - start;
    
    printf("Multi-thread time:  %.6f seconds\n", time_multi);
    printf("Speedup:            %.2fx\n\n", time_single / time_multi);
    
    printf("Verifying single-thread result...\n");
    int passed_single = verify_result(mat_ptrs, result_single, M, N);
    printf("Single-thread test: %s\n\n", passed_single ? "PASSED" : "FAILED");
    
    printf("Comparing multi-thread vs single-thread...\n");
    int passed_multi = compare_results(result_single, result_multi, M, N);
    printf("Multi-thread test:  %s\n\n", passed_multi ? "PASSED" : "FAILED");
    
    for (int i = 0; i < NUM_MATRICES; i++) {
        free(matrices[i]);
    }
    free(matrices);
    free(result_single);
    free(result_multi);
    
    return (passed_single && passed_multi) ? 0 : 1;
}