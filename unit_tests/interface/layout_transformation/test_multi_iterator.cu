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

#include "test_multi_iterator.cpp"

using dim2_ = gridtools::array< size_t, 4 >;
__device__ int get_val(size_t a, size_t b) { return a * 2 + b; }

__global__ void test_exec(dim2_ *out_ptr) {
    gridtools::array< uint_t, 2 > dims{2, 2};
    dim2_ &out = *out_ptr;

    for (size_t i = 0; i < 4; ++i)
        out[i] = 0;

    iterate(dims, [&](size_t a, size_t b) { out[get_val(a, b)] = 1; });
};

TEST(multi_iterator, trying_to_execute_on_device) {

    dim2_ *out;
    cudaMalloc(&out, sizeof(dim2_));

    test_exec<<< 1, 1 >>>(out);

    dim2_ host_out;
    cudaMemcpy(&host_out, out, sizeof(dim2_), cudaMemcpyDeviceToHost);

    for (size_t i = 0; i < 4; ++i)
        ASSERT_EQ(1, host_out[i]);
}
