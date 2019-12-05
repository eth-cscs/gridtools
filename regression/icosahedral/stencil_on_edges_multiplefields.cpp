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

#include <gridtools/stencil_composition/icosahedral.hpp>
#include <gridtools/tools/regression_fixture.hpp>

#include "neighbours_of.hpp"

using namespace gridtools;
using namespace icosahedral;

struct test_on_edges_functor {
    using in1 = in_accessor<0, edges, extent<1, -1, 1, -1>>;
    using in2 = in_accessor<1, edges, extent<1, -1, 1, -1>>;
    using out = inout_accessor<2, edges>;
    using param_list = make_param_list<in1, in2, out>;
    using location = edges;

    template <class Eval>
    GT_FUNCTION static void apply(Eval &&eval) {
        float_type res = 0;
        eval.for_neighbors([&](auto in1, auto in2) { res += in1 + in2 * float_type(.1); }, in1(), in2());
        eval(out()) = res;
    }
};

using stencil_on_edges_multiplefields = regression_fixture<1>;

TEST_F(stencil_on_edges_multiplefields, test) {
    auto in1 = [](int_t i, int_t j, int_t k, int_t c) { return i + j + k + c; };
    auto in2 = [](int_t i, int_t j, int_t k, int_t c) { return i / 2 + j / 2 + k / 2 + c; };
    auto ref = [=](int_t i, int_t j, int_t k, int_t c) {
        float_type res = 0;
        for (auto &&item : neighbours_of<edges, edges>(i, j, k, c))
            res += item.call(in1) + .1 * item.call(in2);
        return res;
    };
    auto out = make_storage<edges>();
    auto comp = [&] {
        easy_run(
            test_on_edges_functor(), backend_t(), make_grid(), make_storage<edges>(in1), make_storage<edges>(in2), out);
    };
    comp();
    verify(ref, out);
    benchmark(comp);
}
