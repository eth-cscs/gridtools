/*
  GridTools Libraries

  Copyright (c) 2017, ETH Zurich and MeteoSwiss
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  For information: http://eth-cscs.github.io/gridtools/
*/
#include "gtest/gtest.h"
#include <cstdlib>
#include "cuda_runtime.h"
#include "gridtools/common/defs.hpp"
#include "gridtools/common/atomic_functions.hpp"

template < typename T >
struct verifier {
    static void TestEQ(T val, T exp) {
        T err = std::fabs(val - exp) / std::fabs(val);
        ASSERT_TRUE(err < 1e-12);
    }
};

template <>
struct verifier< float > {
    static void TestEQ(float val, float exp) {
        double err = std::fabs(val - exp) / std::fabs(val);
        ASSERT_TRUE(err < 1e-6);
    }
};

template <>
struct verifier< int > {
    static void TestEQ(int val, int exp) { ASSERT_EQ(val, exp); }
};

template < typename T >
__global__ void atomic_add_kernel(T *pReduced, const T *field, const int size) {
    const int i = threadIdx.x + blockIdx.x * blockDim.x;
    const int j = threadIdx.y + blockIdx.y * blockDim.y;
    const int pos = j * gridDim.x * blockDim.x + i;
    gridtools::atomic_add(*pReduced, field[pos]);
}

template < typename T >
__global__ void atomic_sub_kernel(T *pReduced, const T *field, const int size) {
    const int i = threadIdx.x + blockIdx.x * blockDim.x;
    const int j = threadIdx.y + blockIdx.y * blockDim.y;
    const int pos = j * gridDim.x * blockDim.x + i;
    gridtools::atomic_sub(*pReduced, field[pos]);
}
template < typename T >
__global__ void atomic_min_kernel(T *pReduced, const T *field, const int size) {
    const int i = threadIdx.x + blockIdx.x * blockDim.x;
    const int j = threadIdx.y + blockIdx.y * blockDim.y;
    const int pos = j * gridDim.x * blockDim.x + i;
    gridtools::atomic_min(*pReduced, field[pos]);
}
template < typename T >
__global__ void atomic_max_kernel(T *pReduced, const T *field, const int size) {
    const int i = threadIdx.x + blockIdx.x * blockDim.x;
    const int j = threadIdx.y + blockIdx.y * blockDim.y;
    const int pos = j * gridDim.x * blockDim.x + i;
    gridtools::atomic_max(*pReduced, field[pos]);
}

template < typename T >
void test_atomic_add() {
    dim3 threadsPerBlock(4, 4);
    dim3 numberOfBlocks(4, 4);

    int size = threadsPerBlock.x * threadsPerBlock.y * numberOfBlocks.x * numberOfBlocks.y;
    T field[size];

    T sumRef = 0;
    T sum = 0;
    T *sumDevice;
    cudaMalloc(&sumDevice, sizeof(T));
    cudaMemcpy(sumDevice, &sum, sizeof(T), cudaMemcpyHostToDevice);

    T *fieldDevice;
    cudaMalloc(&fieldDevice, sizeof(T) * size);

    for (int cnt = 0; cnt < size; ++cnt) {
        field[cnt] = static_cast< T >(std::rand() % 100 + (std::rand() % 100) * 0.005);
        sumRef += field[cnt];
    }

    cudaMemcpy(fieldDevice, &field[0], sizeof(T) * size, cudaMemcpyHostToDevice);

    // clang-format off
    atomic_add_kernel<<<numberOfBlocks, threadsPerBlock>>>(sumDevice, fieldDevice, size);
    // clang-format on

    cudaMemcpy(&sum, sumDevice, sizeof(T), cudaMemcpyDeviceToHost);
    verifier< T >::TestEQ(sumRef, sum);
}

template < typename T >
void test_atomic_sub() {
    dim3 threadsPerBlock(4, 4);
    dim3 numberOfBlocks(4, 4);

    int size = threadsPerBlock.x * threadsPerBlock.y * numberOfBlocks.x * numberOfBlocks.y;
    T field[size];

    T sumRef = 0;
    T sum = 0;
    T *sumDevice;
    cudaMalloc(&sumDevice, sizeof(T));
    cudaMemcpy(sumDevice, &sum, sizeof(T), cudaMemcpyHostToDevice);

    T *fieldDevice;
    cudaMalloc(&fieldDevice, sizeof(T) * size);

    for (int cnt = 0; cnt < size; ++cnt) {
        field[cnt] = static_cast< T >(std::rand() % 100 + (std::rand() % 100) * 0.005);
        sumRef -= field[cnt];
    }

    cudaMemcpy(fieldDevice, &field[0], sizeof(T) * size, cudaMemcpyHostToDevice);

    // clang-format off
    atomic_sub_kernel<<<numberOfBlocks, threadsPerBlock>>>(sumDevice, fieldDevice, size);
    // clang-format on

    cudaMemcpy(&sum, sumDevice, sizeof(T), cudaMemcpyDeviceToHost);
    verifier< T >::TestEQ(sumRef, sum);
}

template < typename T >
void test_atomic_min() {
    dim3 threadsPerBlock(4, 4);
    dim3 numberOfBlocks(4, 4);

    int size = threadsPerBlock.x * threadsPerBlock.y * numberOfBlocks.x * numberOfBlocks.y;
    T field[size];

    T minRef = 99999;
    T min = 99999;
    T *minDevice;
    cudaMalloc(&minDevice, sizeof(T));
    cudaMemcpy(minDevice, &min, sizeof(T), cudaMemcpyHostToDevice);

    T *fieldDevice;
    cudaMalloc(&fieldDevice, sizeof(T) * size);

    for (int cnt = 0; cnt < size; ++cnt) {
        field[cnt] = static_cast< T >(std::rand() % 100 + (std::rand() % 100) * 0.005);
        minRef = std::min(minRef, field[cnt]);
    }

    cudaMemcpy(fieldDevice, &field[0], sizeof(T) * size, cudaMemcpyHostToDevice);

    // clang-format off
    atomic_min_kernel<<<numberOfBlocks, threadsPerBlock>>>(minDevice, fieldDevice, size);
    // clang-format on

    cudaMemcpy(&min, minDevice, sizeof(T), cudaMemcpyDeviceToHost);
    verifier< T >::TestEQ(minRef, min);
}
template < typename T >
void test_atomic_max() {
    dim3 threadsPerBlock(4, 4);
    dim3 numberOfBlocks(4, 4);

    int size = threadsPerBlock.x * threadsPerBlock.y * numberOfBlocks.x * numberOfBlocks.y;
    T field[size];

    T maxRef = -1;
    T max = -1;
    T *maxDevice;
    cudaMalloc(&maxDevice, sizeof(T));
    cudaMemcpy(maxDevice, &max, sizeof(T), cudaMemcpyHostToDevice);

    T *fieldDevice;
    cudaMalloc(&fieldDevice, sizeof(T) * size);

    for (int cnt = 0; cnt < size; ++cnt) {
        field[cnt] = static_cast< T >(std::rand() % 100 + (std::rand() % 100) * 0.005);
        maxRef = std::max(maxRef, field[cnt]);
    }

    cudaMemcpy(fieldDevice, &field[0], sizeof(T) * size, cudaMemcpyHostToDevice);

    // clang-format off
    atomic_max_kernel<<<numberOfBlocks, threadsPerBlock>>>(maxDevice, fieldDevice, size);
    // clang-format on

    cudaMemcpy(&max, maxDevice, sizeof(T), cudaMemcpyDeviceToHost);
    verifier< T >::TestEQ(maxRef, max);
}

TEST(AtomicFunctionsUnittest, atomic_add_int) { test_atomic_add< int >(); }

TEST(AtomicFunctionsUnittest, atomic_add_real) {
    test_atomic_add< double >();
    test_atomic_add< float >();
}

TEST(AtomicFunctionsUnittest, atomic_sub_int) { test_atomic_sub< int >(); }

TEST(AtomicFunctionsUnittest, atomic_sub_real) {
    test_atomic_sub< double >();
    test_atomic_sub< float >();
}

TEST(AtomicFunctionsUnittest, atomic_min_int) { test_atomic_min< int >(); }
TEST(AtomicFunctionsUnittest, atomic_min_real) {
    test_atomic_min< double >();
    test_atomic_min< float >();
}

TEST(AtomicFunctionsUnittest, atomic_max_int) { test_atomic_max< int >(); }
TEST(AtomicFunctionsUnittest, atomic_max_real) {
    test_atomic_max< double >();
    test_atomic_max< float >();
}
