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

#include <gridtools/fn/cartesian2.hpp>
#include <gridtools/fn/unstructured2.hpp>

#include <fn_select.hpp>
#include <test_environment.hpp>

namespace {
    using namespace gridtools;
    using namespace fn;
    using namespace literals;

    struct forward_scan : fwd {
        static GT_FUNCTION constexpr auto prologue() {
            return tuple(scan_pass(
                [](auto /*acc*/, auto const & /*a*/, auto const &b, auto const &c, auto const &d) {
                    return tuple(deref(c) / deref(b), deref(d) / deref(b));
                },
                identity()));
        }

        static GT_FUNCTION constexpr auto body() {
            return scan_pass(
                [](auto acc, auto const &a, auto const &b, auto const &c, auto const &d) {
                    auto [cp, dp] = acc;
                    return tuple(
                        deref(c) / (deref(b) - deref(a) * cp), (deref(d) - deref(a) * dp) / (deref(b) - deref(a) * cp));
                },
                identity());
        }
    };

    struct backward_scan : bwd {
        static GT_FUNCTION constexpr auto prologue() {
            return tuple(scan_pass(
                [](auto /*xp*/, auto const &cpdp) {
                    auto [cp, dp] = deref(cpdp);
                    return dp;
                },
                identity()));
        }

        static GT_FUNCTION constexpr auto body() {
            return scan_pass(
                [](auto xp, auto const &cpdp) {
                    auto [cp, dp] = deref(cpdp);
                    return dp - cp * xp;
                },
                identity());
        }
    };

    GT_REGRESSION_TEST(fn_cartesian_tridiagonal_solve, vertical_test_environment<>, fn_backend_t) {
        using float_t = typename TypeParam::float_t;

        auto a = [](int, int, int) -> float_t { return -1; };
        auto b = [](int, int, int) -> float_t { return 3; };
        auto c = [](int, int, int) -> float_t { return 1; };
        auto d = [kmax = TypeParam::d(2) - 1](int, int, int k) -> float_t { return k == 0 ? 4 : k == kmax ? 2 : 3; };
        auto expected = [](int, int, int) -> float_t { return 1; };

        auto tridiagonal_solve =
            [](auto executor, auto const &a, auto const &b, auto const &c, auto const &d, auto &cpdp, auto &x) {
                return executor()
                    .arg(a)
                    .arg(b)
                    .arg(c)
                    .arg(d)
                    .arg(cpdp)
                    .arg(x)
                    .assign(4_c, forward_scan(), tuple<float_t, float_t>(0, 0), 0_c, 1_c, 2_c, 3_c)
                    .assign(5_c, backward_scan(), float_t(0), 4_c);
            };

        auto tridiagonal_solve_fencil =
            [&](auto sizes, auto const &a, auto const &b, auto const &c, auto const &d, auto &x) {
                auto domain = cartesian_domain(sizes);
                auto backend = make_backend(fn_backend_t(), domain);
                auto cpdp = backend.template make_tmp<tuple<float_t, float_t>>();
                tridiagonal_solve(backend.vertical_executor(), a, b, c, d, cpdp, x);
            };

        auto x = TypeParam::make_storage();
        auto comp = [&,
                        a = TypeParam::make_const_storage(a),
                        b = TypeParam::make_const_storage(b),
                        c = TypeParam::make_const_storage(c),
                        d = TypeParam::make_const_storage(d)] {
            tridiagonal_solve_fencil(TypeParam::fn_cartesian_sizes(), a, b, c, d, x);
        };
        comp();
        TypeParam::verify(expected, x);
    }
} // namespace
