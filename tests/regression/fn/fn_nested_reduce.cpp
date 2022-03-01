/*
 * GridTools
 *
 * Copyright (c) 2014-2021, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <gtest/gtest.h>

#include <gridtools/fn.hpp>
#include <gridtools/fn/backend/naive.hpp>

#include "simple_mesh.hpp"

using namespace gridtools;
using namespace fn;
using namespace literals;

constexpr auto zero = []<class T>(T) { return T{}; };
constexpr auto sum = reduce<plus, zero>;

template <auto V2V>
constexpr auto nb_sum = [](auto const &x) { return sum(shift<V2V>(x)); };

template <auto V2V, bool UseTmp>
constexpr auto nb_nb_sum = [](auto const &x) { return sum(shift<V2V>(lift<sum, UseTmp>(shift<V2V>(x)))); };

template <auto V2V, bool UseTmp>
constexpr auto nb_nb_nb_sum =
    [](auto const &x) { return sum(shift<V2V>(lift<sum, UseTmp>(shift<V2V>(lift<sum, UseTmp>(shift<V2V>(x)))))); };

using namespace simple_mesh;
constexpr auto K = 3_c;
constexpr auto n_v2v = neighbours_num<decltype(v2v)>::value;

using params_t = testing::Types<std::false_type, std::true_type>;

template <class>
using lift_test = testing::Test;

TYPED_TEST_SUITE(lift_test, params_t);

TYPED_TEST(lift_test, nb_sum) {
    double x[n_vertices][K];
    for (auto &xx : x)
        for (auto &xxx : xx)
            xxx = rand() % 100;

    auto expected = [&](int h, int v) {
        double res = 0.0;
        for (int i = 0; i < n_v2v; ++i) {
            auto vi = v2v[h][i];
            if (vi != -1)
                res += x[vi][v];
        }
        return res;
    };

    double actual[n_vertices][K] = {};

    using stage_t = make_stage<nb_sum<v2v>, std::identity{}, 0, 1>;
    constexpr auto testee = fencil<naive, stage_t>;
    constexpr auto domain = unstructured(std::tuple(n_vertices, K));
    testee(domain, actual, x);

    for (int h = 0; h < n_vertices; ++h)
        for (int v = 0; v < K; ++v)
            EXPECT_DOUBLE_EQ(actual[h][v], expected(h, v));
}

TYPED_TEST(lift_test, nb_nb_sum) {
    double x[n_vertices][K];
    for (auto &xx : x)
        for (auto &xxx : xx)
            xxx = rand() % 100;

    auto expected = [&](int h, int v) {
        double res = 0.0;
        for (int i = 0; i < n_v2v; ++i) {
            auto vi = v2v[h][i];
            if (vi != -1) {
                for (int j = 0; j < n_v2v; ++j) {
                    auto vj = v2v[vi][j];
                    if (vj != -1)
                        res += x[vj][v];
                }
            }
        }
        return res;
    };

    double actual[n_vertices][K] = {};

    using stage_t = make_stage<nb_nb_sum<v2v, TypeParam::value>, std::identity{}, 0, 1>;
    constexpr auto testee = fencil<naive, stage_t>;
    constexpr auto domain = unstructured(std::tuple(n_vertices, K));
    testee(domain, actual, x);

    for (int h = 0; h < n_vertices; ++h)
        for (int v = 0; v < K; ++v)
            EXPECT_DOUBLE_EQ(actual[h][v], expected(h, v));
}

TYPED_TEST(lift_test, nb_nb_nb_sum) {
    double x[n_vertices][K];
    for (auto &xx : x)
        for (auto &xxx : xx)
            xxx = rand() % 100;

    auto expected = [&](int h, int v) {
        double res = 0.0;
        for (int i = 0; i < n_v2v; ++i) {
            auto vi = v2v[h][i];
            if (vi != -1) {
                for (int j = 0; j < n_v2v; ++j) {
                    auto vj = v2v[vi][j];
                    if (vj != -1) {
                        for (int k = 0; k < n_v2v; ++k) {
                            auto vk = v2v[vj][k];
                            if (vk != -1)
                                res += x[vk][v];
                        }
                    }
                }
            }
        }
        return res;
    };

    double actual[n_vertices][K] = {};

    using stage_t = make_stage<nb_nb_nb_sum<v2v, TypeParam::value>, std::identity{}, 0, 1>;
    constexpr auto testee = fencil<naive, stage_t>;
    constexpr auto domain = unstructured(std::tuple(n_vertices, K));
    testee(domain, actual, x);

    for (int h = 0; h < n_vertices; ++h)
        for (int v = 0; v < K; ++v)
            EXPECT_DOUBLE_EQ(actual[h][v], expected(h, v));
}
