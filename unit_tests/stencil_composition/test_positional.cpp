/*
 * GridTools
 *
 * Copyright (c) 2014-2019, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <gridtools/stencil_composition/positional.hpp>

#include <gtest/gtest.h>

#include <gridtools/stencil_composition/dim.hpp>
#include <gridtools/stencil_composition/sid/concept.hpp>

namespace gridtools {
    namespace {
        struct d;
        using testee_t = positional<d>;

        static_assert(is_sid<testee_t>(), "");

        TEST(positional, smoke) {
            testee_t testee{1};

            auto ptr = sid::get_origin(testee)();

            EXPECT_EQ(*ptr, 1);

            auto strides = sid::get_strides(testee);

            sid::shift(ptr, sid::get_stride<d>(strides), -34);

            EXPECT_EQ(*ptr, -33);

            using diff_t = GT_META_CALL(sid::ptr_diff_type, testee_t);

            diff_t diff{};

            sid::shift(diff, sid::get_stride<d>(strides), -34);

            ptr = sid::get_origin(testee)() + diff;

            EXPECT_EQ(*ptr, -33);
        }
    } // namespace
} // namespace gridtools
