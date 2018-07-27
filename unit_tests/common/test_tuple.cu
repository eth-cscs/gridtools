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
#include "test_tuple.cpp"

__global__ void test_tuple_kernel(bool *result) {
    *result = true;

    auto t1 = gridtools::make_tuple(0.0, 1, 2.0f);
    auto t2 = gridtools::make_tuple(3.0, 4, 5.0f);

    *result &= get<0>(t1) == 0.0;
    *result &= get<1>(t1) == 1;
    *result &= get<2>(t1) == 2.0f;
    *result &= get<0>(t2) == 3.0;
    *result &= get<1>(t2) == 4;
    *result &= get<2>(t2) == 5.0f;

    t1.swap(t2);

    *result &= get<0>(t1) == 3.0;
    *result &= get<1>(t1) == 4;
    *result &= get<2>(t1) == 5.0f;
    *result &= get<0>(t2) == 0.0;
    *result &= get<1>(t2) == 1;
    *result &= get<2>(t2) == 2.0f;

    get<0>(t1) = 0.0;
    get<1>(t1) = 1;
    get<2>(t1) = 2.0f;

    *result &= get<0>(t1) == 0.0;
    *result &= get<1>(t1) == 1;
    *result &= get<2>(t1) == 2.0f;
}

TEST(tuple, test_on_device) {
    bool result;
    bool *resultDevice;
    cudaMalloc(&resultDevice, sizeof(bool));

    test_tuple_kernel<<<1, 1>>>(resultDevice);

    cudaMemcpy(&result, resultDevice, sizeof(bool), cudaMemcpyDeviceToHost);
    ASSERT_TRUE(result);
}
