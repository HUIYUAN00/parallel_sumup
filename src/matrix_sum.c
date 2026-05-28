#include "matrix_sum.h"
#include <string.h>

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