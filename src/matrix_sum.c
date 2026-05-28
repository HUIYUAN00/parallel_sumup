#include "matrix_sum.h"
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

void matrix_sum_single_thread(float** matrices, size_t M, size_t N) {
    size_t total = M * N;
    float* result = matrices[0];
    
    for (size_t k = 1; k < NUM_MATRICES; k++) {
        const float* mat = matrices[k];
        for (size_t i = 0; i < total; i++) {
            result[i] += mat[i];
        }
    }
}

typedef struct {
    float** matrices;
    size_t M;
    size_t N;
    int start_idx;
    int count;
} ThreadArg;

static void* thread_sum(void* arg) {
    ThreadArg* ta = (ThreadArg*)arg;
    size_t total = ta->M * ta->N;
    float* target = ta->matrices[ta->start_idx];
    
    for (int k = 1; k < ta->count; k++) {
        const float* mat = ta->matrices[ta->start_idx + k];
        for (size_t i = 0; i < total; i++) {
            target[i] += mat[i];
        }
    }
    
    return NULL;
}

void matrix_sum_multi_thread(float** matrices, size_t M, size_t N, int num_threads) {
    size_t total = M * N;
    
    if (num_threads <= 1 || num_threads > NUM_MATRICES) {
        matrix_sum_single_thread(matrices, M, N);
        return;
    }
    
    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    ThreadArg* args = (ThreadArg*)malloc(num_threads * sizeof(ThreadArg));
    
    int matrices_per_thread = NUM_MATRICES / num_threads;
    int remainder = NUM_MATRICES % num_threads;
    
    int current_idx = 0;
    int group_start_indices[num_threads];
    
    for (int t = 0; t < num_threads; t++) {
        int count = matrices_per_thread;
        if (t < remainder) {
            count++;
        }
        
        args[t].matrices = matrices;
        args[t].M = M;
        args[t].N = N;
        args[t].start_idx = current_idx;
        args[t].count = count;
        
        group_start_indices[t] = current_idx;
        
        pthread_create(&threads[t], NULL, thread_sum, &args[t]);
        current_idx += count;
    }
    
    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }
    
    float* result = matrices[0];
    for (int t = 1; t < num_threads; t++) {
        const float* group_result = matrices[group_start_indices[t]];
        for (size_t i = 0; i < total; i++) {
            result[i] += group_result[i];
        }
    }
    
    free(threads);
    free(args);
}

typedef struct {
    float** matrices;
    int target_idx;
    int source_idx;
    size_t total;
} PairSumArg;

static void* thread_pair_sum(void* arg) {
    PairSumArg* pa = (PairSumArg*)arg;
    float* target = pa->matrices[pa->target_idx];
    const float* source = pa->matrices[pa->source_idx];
    
    for (size_t i = 0; i < pa->total; i++) {
        target[i] += source[i];
    }
    
    return NULL;
}

void matrix_sum_divide_conquer(float** matrices, size_t M, size_t N, int num_threads) {
    size_t total = M * N;
    
    int active[NUM_MATRICES];
    for (int i = 0; i < NUM_MATRICES; i++) {
        active[i] = 1;
    }
    
    int active_count = NUM_MATRICES;
    int round = 0;
    
    while (active_count > 1) {
        round++;
        
        int active_indices[NUM_MATRICES];
        int idx = 0;
        for (int i = 0; i < NUM_MATRICES; i++) {
            if (active[i]) {
                active_indices[idx++] = i;
            }
        }
        
        int pairs = active_count / 2;
        int remainder = active_count % 2;
        
        int threads_to_use = (pairs < num_threads) ? pairs : num_threads;
        
        if (threads_to_use <= 0) {
            for (int p = 0; p < pairs; p++) {
                int target = active_indices[p * 2];
                int source = active_indices[p * 2 + 1];
                float* target_mat = matrices[target];
                const float* source_mat = matrices[source];
                for (size_t i = 0; i < total; i++) {
                    target_mat[i] += source_mat[i];
                }
                active[source] = 0;
            }
        } else {
            pthread_t* threads = (pthread_t*)malloc(threads_to_use * sizeof(pthread_t));
            PairSumArg* args = (PairSumArg*)malloc(pairs * sizeof(PairSumArg));
            
            for (int p = 0; p < pairs; p++) {
                args[p].matrices = matrices;
                args[p].target_idx = active_indices[p * 2];
                args[p].source_idx = active_indices[p * 2 + 1];
                args[p].total = total;
            }
            
            int batch = 0;
            while (batch < pairs) {
                int batch_size = (pairs - batch < threads_to_use) ? (pairs - batch) : threads_to_use;
                
                for (int t = 0; t < batch_size; t++) {
                    pthread_create(&threads[t], NULL, thread_pair_sum, &args[batch + t]);
                }
                
                for (int t = 0; t < batch_size; t++) {
                    pthread_join(threads[t], NULL);
                }
                
                batch += batch_size;
            }
            
            for (int p = 0; p < pairs; p++) {
                active[active_indices[p * 2 + 1]] = 0;
            }
            
            free(threads);
            free(args);
        }
        
        active_count = pairs + remainder;
    }
}