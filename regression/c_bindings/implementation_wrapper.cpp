/*
 * GridTools
 *
 * Copyright (c) 2014-2019, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cpp_bindgen/export.hpp>

#include <gridtools/common/hymap.hpp>
#include <gridtools/interface/fortran_array_view.hpp>
#include <gridtools/stencil_composition/cartesian.hpp>
#include <gridtools/tools/backend_select.hpp>

namespace {
    using namespace gridtools;
    using namespace cartesian;

    struct copy_functor {
        using in = in_accessor<0>;
        using out = inout_accessor<1>;
        using param_list = make_param_list<in, out>;

        template <typename Evaluation>
        GT_FUNCTION static void apply(Evaluation &eval) {
            eval(out{}) = eval(in{});
        }
    };

    template <class T>
    void run_copy_functor_impl(fortran_array_view<T, 3> in, fortran_array_view<T, 3> out) {
        auto &&size = sid::get_upper_bounds(out);
        easy_run(copy_functor(),
            backend_t(),
            make_grid(at_key<dim::i>(size), at_key<dim::j>(size), at_key<dim::k>(size)),
            in,
            out);
    }
    BINDGEN_EXPORT_GENERIC_BINDING_WRAPPED(2, run_copy_functor, run_copy_functor_impl, (double)(float));
} // namespace
