/*
 * GridTools
 *
 * Copyright (c) 2014-2023, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <gridtools/fn/sid_neighbor_table.hpp>

#include <array>
#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

namespace gridtools::fn {
    namespace {

        using sid_neighbor_table::as_neighbor_table;

        struct edge_dim_t{};
        struct edge_to_cell_dim_t{};

        TEST(sid_neighbor_table, correctness) {
            constexpr std::size_t num_elements = 3;
            constexpr std::size_t num_neighbors = 2;
            const int contents[num_elements][num_neighbors] = {{0, 1}, {10, 11}, {20, 21}};
            const auto table = as_neighbor_table<edge_dim_t, edge_to_cell_dim_t, num_neighbors>(contents);

            auto [n00, n01] = neighbor_table::neighbors(table, 0);
            auto [n10, n11] = neighbor_table::neighbors(table, 1);
            auto [n20, n21] = neighbor_table::neighbors(table, 2);
            EXPECT_EQ(n00, 0);
            EXPECT_EQ(n01, 1);
            EXPECT_EQ(n10, 10);
            EXPECT_EQ(n11, 11);
            EXPECT_EQ(n20, 20);
            EXPECT_EQ(n21, 21);
        }

    } // namespace
} // namespace gridtools::fn
