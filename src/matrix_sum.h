#ifndef MATRIX_SUM_H
#define MATRIX_SUM_H

#include <stddef.h>

#define NUM_MATRICES 36

void matrix_sum_single_thread(const float** matrices, float* result, size_t M, size_t N);

void matrix_sum_multi_thread(const float** matrices, float* result, size_t M, size_t N, int num_threads);

#endif