/*
  GridTools Libraries

  Copyright (c) 2016, GridTools Consortium
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
#include "common/defs.hpp"
#include "common/gt_math.hpp"
#include "tools/verifier.hpp"
using namespace gridtools;

struct test_fabs {
    static bool GT_FUNCTION Do() {
        GRIDTOOLS_STATIC_ASSERT((std::is_same< decltype(math::fabs(4.0f)), float >::value), "Should return float.");
        GRIDTOOLS_STATIC_ASSERT((std::is_same< decltype(math::fabs(4.0)), double >::value), "Should return double.");
#ifndef __CUDA_ARCH__
        GRIDTOOLS_STATIC_ASSERT(
            (std::is_same< decltype(math::fabs((long double)4)), long double >::value), "Should return long double.");
#endif
        GRIDTOOLS_STATIC_ASSERT((std::is_same< decltype(math::fabs((int)4)), double >::value), "Should return double.");

        if (!compare_below_threshold(math::fabs(5.6), 5.6, 1e-14))
            return false;
        else if (!compare_below_threshold(math::fabs(-5.6), 5.6, 1e-14))
            return false;
        else if (!compare_below_threshold(math::fabs(-5.6f), 5.6f, 1e-14))
            return false;
        else if (!compare_below_threshold(math::fabs(-5), (double)5, 1e-14))
            return false;
#ifndef __CUDA_ARCH__
        else if (!compare_below_threshold(math::fabs((long double)-5), (long double)5., 1e-14))
            return false;
#endif
        else
            return true;
    }
};

struct test_abs {
    static GT_FUNCTION bool Do() {
        // float overloads
        GRIDTOOLS_STATIC_ASSERT((std::is_same< decltype(math::abs(4.0f)), float >::value), "Should return float.");
        GRIDTOOLS_STATIC_ASSERT((std::is_same< decltype(math::abs(4.0)), double >::value), "Should return double.");
        GRIDTOOLS_STATIC_ASSERT(
            (std::is_same< decltype(math::abs((long double)4)), long double >::value), "Should return long double.");

        // int overloads
        GRIDTOOLS_STATIC_ASSERT((std::is_same< decltype(math::abs((int)4)), int >::value), "Should return int.");
        GRIDTOOLS_STATIC_ASSERT((std::is_same< decltype(math::abs((long)4)), long >::value), "Should return long.");
        GRIDTOOLS_STATIC_ASSERT(
            (std::is_same< decltype(math::abs((long long)4)), long long >::value), "Should return long long.");

        if (math::abs(5.6) != 5.6)
            return false;
        else if (math::abs(-5.6) != 5.6)
            return false;
        else if (math::abs(-5) != 5)
            return false;
        else
            return true;
    }
};

TEST(math, test_min) {
    ASSERT_TRUE(math::min(5, 2, 7) == 2);
    ASSERT_TRUE(math::min(5, -1) == -1);

    ASSERT_REAL_EQ(math::min(5.3, 22.0, 7.7), 5.3);

    // checking returned by const &
    double a = 3.5;
    double b = 2.3;
    double const &min = math::min(a, b);
    ASSERT_REAL_EQ(min, 2.3);
    b = 8;
    ASSERT_REAL_EQ(min, 8);
}

TEST(math, test_max) {
    ASSERT_TRUE(math::max(5, 2, 7) == 7);
    ASSERT_TRUE(math::max(5, -1) == 5);

    ASSERT_REAL_EQ(math::max(5.3, 22.0, 7.7), 22.0);
    // checking returned by const &
    double a = 3.5;
    double b = 2.3;
    double const &max = math::max(a, b);

    ASSERT_REAL_EQ(max, 3.5);
    a = 8;
    ASSERT_REAL_EQ(max, 8);
}

TEST(math, test_fabs) { ASSERT_TRUE(test_fabs::Do()); }

TEST(math, test_abs) { ASSERT_TRUE(test_abs::Do()); }
