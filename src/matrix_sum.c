#include "matrix_sum.h"
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

void matrix_sum_single_thread(const float** matrices, float* result, size_t M, size_t N) {
    size_t total = M * N;
    memset(result, 0, total * sizeof(float));
    
    for (size_t k = 0; k < NUM_MATRICES; k++) {
        const float* mat = matrices[k];
        for (size_t i = 0; i < total; i++) {
            result[i] += mat[i];
        }
    }
}

typedef struct {
    const float** matrices;
    float* result;
    size_t M;
    size_t N;
    int start_idx;
    int count;
} ThreadArg;

static void* thread_sum(void* arg) {
    ThreadArg* ta = (ThreadArg*)arg;
    size_t total = ta->M * ta->N;
    
    memset(ta->result, 0, total * sizeof(float));
    
    for (int k = 0; k < ta->count; k++) {
        const float* mat = ta->matrices[ta->start_idx + k];
        for (size_t i = 0; i < total; i++) {
            ta->result[i] += mat[i];
        }
    }
    
    return NULL;
}

void matrix_sum_multi_thread(const float** matrices, float* result, size_t M, size_t N, int num_threads) {
    size_t total = M * N;
    
    if (num_threads <= 1 || num_threads > NUM_MATRICES) {
        matrix_sum_single_thread(matrices, result, M, N);
        return;
    }
    
    float** partial_results = (float**)malloc(num_threads * sizeof(float*));
    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    ThreadArg* args = (ThreadArg*)malloc(num_threads * sizeof(ThreadArg));
    
    int matrices_per_thread = NUM_MATRICES / num_threads;
    int remainder = NUM_MATRICES % num_threads;
    
    int current_idx = 0;
    for (int t = 0; t < num_threads; t++) {
        partial_results[t] = (float*)malloc(total * sizeof(float));
        
        int count = matrices_per_thread;
        if (t < remainder) {
            count++;
        }
        
        args[t].matrices = matrices;
        args[t].result = partial_results[t];
        args[t].M = M;
        args[t].N = N;
        args[t].start_idx = current_idx;
        args[t].count = count;
        
        pthread_create(&threads[t], NULL, thread_sum, &args[t]);
        current_idx += count;
    }
    
    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }
    
    memset(result, 0, total * sizeof(float));
    for (int t = 0; t < num_threads; t++) {
        for (size_t i = 0; i < total; i++) {
            result[i] += partial_results[t][i];
        }
        free(partial_results[t]);
    }
    
    free(partial_results);
    free(threads);
    free(args);
}