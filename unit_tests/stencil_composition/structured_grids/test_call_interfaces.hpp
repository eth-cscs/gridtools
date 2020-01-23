/*
 * GridTools
 *
 * Copyright (c) 2014-2019, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <gridtools/stencil_composition/cartesian.hpp>
#include <gridtools/tools/cartesian_fixture.hpp>

namespace gridtools {
    namespace cartesian {
        using x_interval = axis<1>::full_interval;

        struct copy_functor {
            typedef in_accessor<0, extent<>, 3> in;
            typedef inout_accessor<1, extent<>, 3> out;
            typedef make_param_list<in, out> param_list;
            template <typename Evaluation>
            GT_FUNCTION static void apply(Evaluation &eval, x_interval) {
                eval(out()) = eval(in());
            }
        };

        struct copy_functor_with_expression {
            typedef in_accessor<0, extent<>, 3> in;
            typedef inout_accessor<1, extent<>, 3> out;
            typedef make_param_list<in, out> param_list;
            template <typename Evaluation>
            GT_FUNCTION static void apply(Evaluation &eval, x_interval) {
                using namespace expressions;
                eval(out()) = eval(in() + 0.);
            }
        };

        struct copy_functor_default_interval {
            typedef in_accessor<0, extent<>, 3> in;
            typedef inout_accessor<1, extent<>, 3> out;
            typedef make_param_list<in, out> param_list;
            template <class Eval>
            GT_FUNCTION static void apply(Eval &&eval) {
                eval(out()) = eval(in());
            }
        };

        struct base_fixture : computation_fixture<1> {
            base_fixture() : computation_fixture<1>(13, 9, 7) {}

            template <class Fun, class... Storages>
            void run_computation(Storages &&... storages) const {
                run_single_stage(Fun(), backend_t(), make_grid(), std::forward<Storages>(storages)...);
            }

            using fun_t = std::function<double(int, int, int)>;

            fun_t input = [](int i, int j, int k) { return i * 100 + j * 10 + k; };

            fun_t shifted = [this](int i, int j, int k) { return input(i + 1, j + 1, k); };
        };
    } // namespace cartesian
} // namespace gridtools