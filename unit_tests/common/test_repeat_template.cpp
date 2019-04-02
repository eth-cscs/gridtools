/*
 * GridTools
 *
 * Copyright (c) 2014-2019, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <gridtools/common/generic_metafunctions/repeat_template.hpp>

#include <type_traits>

#include <gtest/gtest.h>

#include <gridtools/common/defs.hpp>

using namespace gridtools;

template <uint_t...>
struct halo;

static_assert(std::is_same<typename repeat_template_c<5, 3, halo>::type, halo<5, 5, 5>>{}, "");

static_assert(std::is_same<typename repeat_template_c<5, 3, halo, 4, 7, 8>::type, halo<4, 7, 8, 5, 5, 5>>{}, "");

TEST(dummy, dummy) {}
