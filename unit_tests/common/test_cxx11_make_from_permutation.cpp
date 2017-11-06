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

#include <common/make_from_permutation.hpp>

#include <utility>

#include <gtest/gtest.h>

#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/comparison.hpp>
#include <boost/fusion/include/make_vector.hpp>

namespace gridtools {

    using boost::fusion::vector;
    using boost::fusion::make_vector;

    template < typename Res, typename... Args >
    Res testee(Args &&... args) {
        return make_from_permutation< Res >(make_vector(std::forward< Args >(args)...));
    }

    TEST(_, Empty) { EXPECT_TRUE(testee< vector<> >() == make_vector()); }

    TEST(_, One) { EXPECT_TRUE(testee< vector< int > >(42) == make_vector(42)); }

    TEST(_, Functional) {
        using res_t = vector< int, char, double >;
        res_t expected{42, 'a', .1};
        EXPECT_TRUE(testee< res_t >(42, 'a', .1) == expected);
        EXPECT_TRUE(testee< res_t >(42, .1, 'a') == expected);
        EXPECT_TRUE(testee< res_t >('a', 42, .1) == expected);
        EXPECT_TRUE(testee< res_t >('a', .1, 42) == expected);
        EXPECT_TRUE(testee< res_t >(.1, 42, 'a') == expected);
        EXPECT_TRUE(testee< res_t >(.1, 'a', 42) == expected);
    }

    TEST(_, Ref) {
        vector<> src;
        EXPECT_TRUE(make_from_permutation< vector<> >(src) == vector<>{});
    }

    TEST(_, CRef) {
        vector<> const src;
        EXPECT_TRUE(make_from_permutation< vector<> >(src) == vector<>{});
    }
}
