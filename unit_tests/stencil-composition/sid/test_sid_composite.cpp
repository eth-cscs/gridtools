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

#include <gridtools/stencil-composition/sid/composite.hpp>

#include <gtest/gtest.h>

#include <gridtools/common/array.hpp>
#include <gridtools/common/hymap.hpp>
#include <gridtools/common/integral_constant.hpp>
#include <gridtools/common/tuple_util.hpp>
#include <gridtools/stencil-composition/sid/synthetic.hpp>

namespace gridtools {
    namespace {
        using namespace literals;
        using sid::property;
        namespace tu = tuple_util;
        using tu::get;

        struct a;
        struct b;
        struct c;
        struct d;

        TEST(composite, deref) {
            double const src = 42;
            double dst = 0;

            auto testee = tu::make<sid::composite_ctor::keys<a, b>::values>(
                sid::synthetic().set<property::origin>(&src), sid::synthetic().set<property::origin>(&dst));
            static_assert(is_sid<decltype(testee)>(), "");

            auto ptrs = sid::get_origin(testee);
            EXPECT_EQ(&src, at_key<a>(ptrs));
            EXPECT_EQ(&dst, at_key<b>(ptrs));

            auto refs = *ptrs;
            at_key<b>(refs) = at_key<a>(refs);
            EXPECT_EQ(42, dst);
        }

        struct my_strides_kind;

        using dim_i = integral_constant<int, 0>;
        using dim_j = integral_constant<int, 1>;
        using dim_k = integral_constant<int, 2>;

        TEST(composite, functional) {
            double const one[5] = {0, 10, 20, 30, 40};
            double two = -1;
            double three[4][3][5] = {};
            char four[4][3][5] = {};

            auto my_strides = tu::make<array>(1, 5, 15);

            auto testee = tu::make<sid::composite_ctor::keys<a, b, c, d>::values> //
                (                                                                 //
                    sid::synthetic()                                              //
                        .set<property::origin>(&one[0])                           //
                        .set<property::strides>(tuple_util::make<tuple>(1_c))     //
                    ,                                                             //
                    sid::synthetic()                                              //
                        .set<property::origin>(&two)                              //
                    ,                                                             //
                    sid::synthetic()                                              //
                        .set<property::origin>(&three[0][0][0])                   //
                        .set<property::strides>(my_strides)                       //
                        .set<property::strides_kind, my_strides_kind>()           //
                    ,                                                             //
                    sid::synthetic()                                              //
                        .set<property::origin>(&four[0][0][0])                    //
                        .set<property::strides>(my_strides)                       //
                        .set<property::strides_kind, my_strides_kind>()           //
                );
            static_assert(is_sid<decltype(testee)>(), "");

            auto &&strides = sid::get_strides(testee);
            auto &&stride_i = sid::get_stride<dim_i>(strides);

            EXPECT_EQ(1, at_key<a>(stride_i));
            EXPECT_EQ(0, at_key<b>(stride_i));

            auto ptr = sid::get_origin(testee);

            EXPECT_EQ(0, at_key<a>(*ptr));
            EXPECT_EQ(-1, at_key<b>(*ptr));
            EXPECT_EQ(&four[0][0][0], at_key<d>(ptr));

            using ptr_diff_t = GT_META_CALL(sid::ptr_diff_type, decltype(testee));

            ptr_diff_t ptr_diff;
            EXPECT_EQ(0, at_key<a>(ptr_diff));
            EXPECT_EQ(0, at_key<b>(ptr_diff));

            sid::shift(ptr_diff, stride_i, 1);
            EXPECT_EQ(1, at_key<a>(ptr_diff));
            EXPECT_EQ(0, at_key<b>(ptr_diff));

            sid::shift(ptr_diff, stride_i, 2_c);
            EXPECT_EQ(3, at_key<a>(ptr_diff));
            EXPECT_EQ(0, at_key<b>(ptr_diff));

            ptr = ptr + ptr_diff;
            EXPECT_EQ(30, at_key<a>(*ptr));
            EXPECT_EQ(-1, at_key<b>(*ptr));

            *at_key<b>(ptr) = *at_key<a>(ptr);
            EXPECT_EQ(30, at_key<b>(*ptr));

            sid::shift(ptr, stride_i, -2);
            EXPECT_EQ(10, at_key<a>(*ptr));
            EXPECT_EQ(30, at_key<b>(*ptr));

            EXPECT_EQ(&three[0][0][1], at_key<c>(ptr));
            EXPECT_EQ(&four[0][0][1], at_key<d>(ptr));

            sid::shift(ptr, sid::get_stride<dim_j>(strides), 2);
            sid::shift(ptr, sid::get_stride<dim_k>(strides), 3_c);
            EXPECT_EQ(&three[3][2][1], at_key<c>(ptr));
            EXPECT_EQ(&four[3][2][1], at_key<d>(ptr));

            ptr_diff = {};
            sid::shift(ptr_diff, sid::get_stride<dim_i>(strides), 3);
            sid::shift(ptr_diff, sid::get_stride<dim_j>(strides), 2);
            sid::shift(ptr_diff, sid::get_stride<dim_k>(strides), 1);
            ptr = sid::get_origin(testee) + ptr_diff;
            EXPECT_EQ(&three[1][2][3], at_key<c>(ptr));
            EXPECT_EQ(&four[1][2][3], at_key<d>(ptr));
        }

        struct dim_x;
        struct dim_y;
        struct dim_z;

        TEST(composite, custom_dims) {
            double const one[5] = {0, 10, 20, 30, 40};
            auto strides_one = tu::make<hymap::keys<dim_x>::values>(1_c);

            double two = -1;

            double three[4][3][5] = {};
            auto strides_three = tu::make<hymap::keys<dim_z, dim_y, dim_x>::values>(1_c, 5_c, 15_c);

            char four[6][4][5] = {};
            auto strides_four = tu::make<hymap::keys<dim_y, dim_z, dim_x>::values>(1_c, 5_c, 20_c);

            auto testee = tu::make<sid::composite_ctor::keys<a, b, c, d>::values> //
                (                                                                 //
                    sid::synthetic()                                              //
                        .set<property::origin>(&one[0])                           //
                        .set<property::strides>(strides_one)                      //
                    ,                                                             //
                    sid::synthetic()                                              //
                        .set<property::origin>(&two)                              //
                    ,                                                             //
                    sid::synthetic()                                              //
                        .set<property::origin>(&three[0][0][0])                   //
                        .set<property::strides>(strides_three)                    //
                    ,                                                             //
                    sid::synthetic()                                              //
                        .set<property::origin>(&four[0][0][0])                    //
                        .set<property::strides>(strides_four)                     //
                );

            auto &&strides = sid::get_strides(testee);

            auto ptr = sid::get_origin(testee);

            sid::shift(ptr, sid::get_stride<dim_x>(strides), 3_c);
            sid::shift(ptr, sid::get_stride<dim_y>(strides), 2_c);
            sid::shift(ptr, sid::get_stride<dim_z>(strides), 1_c);

            // no-op: there is no dim_i in our sids.
            sid::shift(ptr, sid::get_stride<dim_i>(strides), 1_c);

            EXPECT_EQ(&one[3], at_key<a>(ptr));
            EXPECT_EQ(&two, at_key<b>(ptr));
            EXPECT_EQ(&three[3][2][1], at_key<c>(ptr));
            EXPECT_EQ(&four[3][1][2], at_key<d>(ptr));
        }
    } // namespace
} // namespace gridtools
