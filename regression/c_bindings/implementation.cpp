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
#include <gridtools/stencil_composition/cartesian.hpp>
#include <gridtools/storage/builder.hpp>
#include <gridtools/storage/sid.hpp>
#include <gridtools/tools/backend_select.hpp>

namespace {
    using namespace gridtools;
    using namespace cartesian;

    struct copy_functor {
        using in = in_accessor<0>;
        using out = inout_accessor<1>;
        using param_list = make_param_list<in, out>;

        template <class Eval>
        GT_FUNCTION static void apply(Eval &&eval) {
            eval(out()) = eval(in());
        }
    };

    auto make_data_store(uint_t x, uint_t y, uint_t z) {
        return storage::builder<storage_traits_t>.template type<float_type>().dimensions(x, y, z)();
    }
    BINDGEN_EXPORT_BINDING_3(create_data_store, make_data_store);

    using data_store_t = decltype(make_data_store(0, 0, 0));

    void run(data_store_t const &in, data_store_t const &out) {
        auto lengths = out->lengths();
        easy_run(copy_functor(), backend_t(), make_grid(lengths[0], lengths[1], lengths[2]), in, out);
    }
    BINDGEN_EXPORT_BINDING_2(run_copy_stencil, run);

    void copy_to(data_store_t const &dst, float_type const *src) {
        auto lengths = dst->lengths();
        auto view = dst->host_view();
        for (int i = 0; i < lengths[0]; ++i)
            for (int j = 0; j < lengths[1]; ++j)
                for (int k = 0; k < lengths[2]; ++k)
                    view(i, j, k) = *(src++);
    }
    BINDGEN_EXPORT_BINDING_2(copy_to_data_store, copy_to);

    void copy_from(data_store_t const &src, float_type *dst) {
        auto lengths = src->lengths();
        auto view = src->const_host_view();
        for (int i = 0; i < lengths[0]; ++i)
            for (int j = 0; j < lengths[1]; ++j)
                for (int k = 0; k < lengths[2]; ++k)
                    *(dst++) = view(i, j, k);
    }
    BINDGEN_EXPORT_BINDING_2(copy_from_data_store, copy_from);
} // namespace
