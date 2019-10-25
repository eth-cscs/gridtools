/*
 * GridTools
 *
 * Copyright (c) 2014-2019, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "gtest/gtest.h"

#include <gridtools/common/gt_assert.hpp>
#include <gridtools/storage/storage_cuda/cuda_storage.hpp>

__global__ void initial_check_s1(int *s) {
    GT_ASSERT_OR_THROW((s[0] == 10), "check failed");
    GT_ASSERT_OR_THROW((s[1] == 10), "check failed");
    if (s[0] != 10 || s[1] != 10) {
        s[0] = -1;
        s[1] = -1;
    }
}

__global__ void check_s1(int *s) {
    GT_ASSERT_OR_THROW((s[0] == 10), "check failed");
    GT_ASSERT_OR_THROW((s[1] == 20), "check failed");
    s[0] = 30;
    s[1] = 40;
}

__global__ void check_s2(int *s) {
    GT_ASSERT_OR_THROW((s[0] == 100), "check failed");
    GT_ASSERT_OR_THROW((s[1] == 200), "check failed");
    s[0] = 300;
    s[1] = 400;
}

TEST(StorageHostTest, Simple) {
    // create two storages
    gridtools::cuda_storage<int> s1(2, 0, gridtools::alignment<32>());
    gridtools::cuda_storage<int> s2(2, 0, gridtools::alignment<32>());
    // test the is_storage check
    static_assert(gridtools::is_storage<decltype(s1)>::value, "");
    static_assert(!gridtools::is_storage<int>::value, "");
    // write some values
    s1.get_cpu_ptr()[0] = 10;
    s1.get_cpu_ptr()[1] = 20;
    s2.get_cpu_ptr()[0] = 100;
    s2.get_cpu_ptr()[1] = 200;
    // check if they are there
    EXPECT_EQ(s1.get_cpu_ptr()[1], 20);
    EXPECT_EQ(s1.get_cpu_ptr()[0], 10);
    EXPECT_EQ(s2.get_cpu_ptr()[1], 200);
    EXPECT_EQ(s2.get_cpu_ptr()[0], 100);

    // clone to device
    s1.clone_to_device();
    s2.clone_to_device();
    // assert if the values were not copied correctly and reset values
    check_s1<<<1, 1>>>(s1.get_target_ptr());
    check_s2<<<1, 1>>>(s2.get_target_ptr());
    // clone_back
    s1.clone_from_device();
    s2.clone_from_device();
    // check values
    EXPECT_EQ(s1.get_cpu_ptr()[1], 40);
    EXPECT_EQ(s1.get_cpu_ptr()[0], 30);
    EXPECT_EQ(s2.get_cpu_ptr()[1], 400);
    EXPECT_EQ(s2.get_cpu_ptr()[0], 300);
}
