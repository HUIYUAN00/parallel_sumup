#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static void reset_matrices(float** matrices, float** backup, size_t M, size_t N) {
    size_t total = M * N * sizeof(float);
    for (int i = 0; i < NUM_MATRICES; i++) {
        memcpy(matrices[i], backup[i], total);
    }
}

static int verify_result(float** matrices, float** backup, size_t M, size_t N) {
    size_t total = M * N;
    float* expected = (float*)calloc(total, sizeof(float));
    
    for (size_t k = 0; k < NUM_MATRICES; k++) {
        for (size_t i = 0; i < total; i++) {
            expected[i] += backup[k][i];
        }
    }
    
    const float* result = matrices[0];
    const float epsilon = 1e-3f;
    int passed = 1;
    size_t errors = 0;
    
    for (size_t i = 0; i < total; i++) {
        float diff = result[i] - expected[i];
        if (diff < 0) diff = -diff;
        
        if (diff > epsilon) {
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
    float** backup = (float**)malloc(NUM_MATRICES * sizeof(float*));
    for (int i = 0; i < NUM_MATRICES; i++) {
        matrices[i] = (float*)malloc(M * N * sizeof(float));
        backup[i] = (float*)malloc(M * N * sizeof(float));
    }
    
    srand((unsigned int)time(NULL));
    for (int i = 0; i < NUM_MATRICES; i++) {
        fill_matrix_random(matrices[i], M, N);
        memcpy(backup[i], matrices[i], M * N * sizeof(float));
    }
    
    double start = get_time_ns();
    matrix_sum_single_thread(matrices, M, N);
    double end = get_time_ns();
    double time_single = end - start;
    
    printf("Single-thread time: %.6f seconds\n", time_single);
    
    printf("Verifying single-thread result...\n");
    int passed_single = verify_result(matrices, backup, M, N);
    printf("Single-thread test: %s\n\n", passed_single ? "PASSED" : "FAILED");
    
    reset_matrices(matrices, backup, M, N);
    
    start = get_time_ns();
    matrix_sum_multi_thread(matrices, M, N, num_threads);
    end = get_time_ns();
    double time_multi = end - start;
    
    printf("Multi-thread time:  %.6f seconds\n", time_multi);
    printf("Speedup:            %.2fx\n\n", time_single / time_multi);
    
    printf("Verifying multi-thread result...\n");
    int passed_multi = verify_result(matrices, backup, M, N);
    printf("Multi-thread test:  %s\n\n", passed_multi ? "PASSED" : "FAILED");
    
    reset_matrices(matrices, backup, M, N);
    
    start = get_time_ns();
    matrix_sum_divide_conquer(matrices, M, N, num_threads);
    end = get_time_ns();
    double time_divide = end - start;
    
    printf("Divide-conquer time: %.6f seconds\n", time_divide);
    printf("Speedup:             %.2fx\n\n", time_single / time_divide);
    
    printf("Verifying divide-conquer result...\n");
    int passed_divide = verify_result(matrices, backup, M, N);
    printf("Divide-conquer test: %s\n\n", passed_divide ? "PASSED" : "FAILED");
    
    for (int i = 0; i < NUM_MATRICES; i++) {
        free(matrices[i]);
        free(backup[i]);
    }
    free(matrices);
    free(backup);
    
    return (passed_single && passed_multi && passed_divide) ? 0 : 1;
}