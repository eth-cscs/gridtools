/*
 * GridTools
 *
 * Copyright (c) 2014-2019, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <gtest/gtest.h>

#include <gridtools/stencil_composition/stencil_composition.hpp>
#include <gridtools/tools/computation_fixture.hpp>

namespace gridtools {
    namespace {
        struct copy_functor {
            using in = in_accessor<0>;
            using out = inout_accessor<1>;

            using param_list = make_param_list<in, out>;

            template <class Eval>
            GT_FUNCTION static void apply(Eval &&eval) {
                eval(out()) = eval(in());
            }
        };

        template <int...>
        struct l_type {};

        template <int... Is>
        constexpr l_type<Is...> l = {};

        struct stencils : computation_fixture<> {
            static constexpr uint_t i_max = 58;
            static constexpr uint_t j_max = 46;
            static constexpr uint_t k_max = 70;
            stencils() : computation_fixture<>(i_max + 1, j_max + 1, k_max + 1) {}

            template <int... Srcs, int... Dsts, class Expected>
            void do_test(l_type<Srcs...>, l_type<Dsts...>, Expected const &expected) {
                auto out = builder().layout<Dsts...>()();
                easy_run(copy_functor(),
                    backend_t(),
                    make_grid(),
                    builder().layout<Srcs...>().initializer([](int i, int j, int k) { return i + j + k; })(),
                    out);
                verify(expected, out);
            }

            template <class Src, class Expected>
            void do_test(Src src, Expected const &expected) {
                do_test(src, l<0, 1, 2>, expected);
            }
        };

        TEST_F(stencils, copies3D) {
            do_test(l<0, 1, 2>, [](int i, int j, int k) { return i + j + k; });
        }
        TEST_F(stencils, copies3Dtranspose) {
            do_test(l<2, 1, 0>, [](int i, int j, int k) { return i + j + k; });
        }

        TEST_F(stencils, copies2Dij) {
            do_test(l<0, 1, -1>, [](int i, int j, int) { return i + j + k_max; });
        }

        TEST_F(stencils, copies2Dik) {
            do_test(l<0, -1, 1>, [](int i, int, int k) { return i + j_max + k; });
        }

        TEST_F(stencils, copies2Djk) {
            do_test(l<-1, 0, 1>, [](int, int j, int k) { return i_max + j + k; });
        }

        TEST_F(stencils, copies1Di) {
            do_test(l<0, -1, -1>, [](int i, int, int) { return i + j_max + k_max; });
        }

        TEST_F(stencils, copies1Dj) {
            do_test(l<-1, 0, -1>, [](int, int j, int) { return i_max + j + k_max; });
        }

        TEST_F(stencils, copies1Dk) {
            do_test(l<-1, -1, 0>, [](int, int, int k) { return i_max + j_max + k; });
        }

        TEST_F(stencils, copiesScalar) {
            do_test(l<-1, -1, -1>, [](int, int, int) { return i_max + j_max + k_max; });
        }

        TEST_F(stencils, copies3DDst) {
            do_test(l<0, 1, 2>, l<2, 0, 1>, [](int i, int j, int k) { return i + j + k; });
        }

        TEST_F(stencils, copies3DtransposeDst) {
            do_test(l<2, 1, 0>, l<2, 0, 1>, [](int i, int j, int k) { return i + j + k; });
        }

// icc hangs here in debug mode
#if !defined(NDEBUG) && (!defined(__INTEL_COMPILER) || __INTEL_COMPILER >= 1900)
        TEST_F(stencils, copies2DijDst) {
            do_test(l<1, 0, -1>, l<2, 0, 1>, [](int i, int j, int) { return i + j + k_max; });
        }

        TEST_F(stencils, copies2DikDst) {
            do_test(l<1, -1, 0>, l<2, 0, 1>, [](int i, int, int k) { return i + j_max + k; });
        }

        TEST_F(stencils, copies2DjkDst) {
            do_test(l<-1, 1, 0>, l<2, 0, 1>, [](int, int j, int k) { return i_max + j + k; });
        }

        TEST_F(stencils, copies2DiDst) {
            do_test(l<0, -1, -1>, l<2, 0, 1>, [](int i, int, int) { return i + j_max + k_max; });
        }

        TEST_F(stencils, copies2DjDst) {
            do_test(l<-1, 0, -1>, l<2, 0, 1>, [](int, int j, int) { return i_max + j + k_max; });
        }

        TEST_F(stencils, copies2DkDst) {
            do_test(l<-1, -1, 0>, l<2, 0, 1>, [](int, int, int k) { return i_max + j_max + k; });
        }

        TEST_F(stencils, copies2DScalarDst) {
            do_test(l<-1, -1, -1>, l<2, 0, 1>, [](int, int, int) { return i_max + j_max + k_max; });
        }
#endif
    } // namespace
} // namespace gridtools
