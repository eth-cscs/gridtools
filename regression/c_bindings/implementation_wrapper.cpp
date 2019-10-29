/*
 * GridTools
 *
 * Copyright (c) 2014-2019, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <functional>
#include <iostream>
#include <typeinfo>

#include <cpp_bindgen/export.hpp>
#include <gridtools/stencil_composition/stencil_composition.hpp>
#include <gridtools/storage/storage_facility.hpp>
#include <gridtools/tools/backend_select.hpp>

namespace {
    using namespace gridtools;

    struct copy_functor {
        using in = in_accessor<0>;
        using out = inout_accessor<1>;
        using param_list = make_param_list<in, out>;

        template <typename Evaluation>
        GT_FUNCTION static void apply(Evaluation &eval) {
            eval(out{}) = eval(in{});
        }
    };

    using storage_info_t = storage_traits<backend_t>::storage_info_t<0, 3>;

    template <class T>
    using generic_data_store_t = storage_traits<backend_t>::data_store_t<T, storage_info_t>;

    using data_store_t = generic_data_store_t<float_type>;
} // namespace

namespace gridtools {
    template <typename T, typename = std::enable_if_t<is_data_store<std::remove_const_t<T>>::value>>
    T bindgen_make_fortran_array_view(bindgen_fortran_array_descriptor *descriptor, T *) {
        if (descriptor->rank != 3) {
            throw std::runtime_error("only 3-dimensional arrays are supported");
        }
        return T(storage_info_t(descriptor->dims[0], descriptor->dims[1], descriptor->dims[2]),
            reinterpret_cast<typename T::data_t *>(descriptor->data));
    }
    template <typename T, typename = std::enable_if_t<is_data_store<std::remove_const_t<T>>::value>>
    bindgen_fortran_array_descriptor get_fortran_view_meta(T *) {
        bindgen_fortran_array_descriptor descriptor;
        descriptor.type = cpp_bindgen::fortran_array_element_kind<typename T::data_t>::value;
        descriptor.rank = 3;
        descriptor.is_acc_present = false;
        return descriptor;
    }

    static_assert(cpp_bindgen::is_fortran_array_bindable<generic_data_store_t<double>>::value, "");
    static_assert(cpp_bindgen::is_fortran_array_wrappable<generic_data_store_t<double>>::value, "");
} // namespace gridtools
namespace {
    BINDGEN_EXPORT_BINDING_WITH_SIGNATURE_WRAPPED_1(
        sync_data_store, void(data_store_t), std::mem_fn(&data_store_t::sync));

    auto make_grid(data_store_t data_store) {
        auto dims = data_store.lengths();
        return gridtools::make_grid(dims[0], dims[1], dims[2]);
    }

    auto make_copy_stencil(data_store_t in, data_store_t out) {
        return [=] { easy_run(copy_functor(), backend_t(), make_grid(out), in, out); };
    }
    BINDGEN_EXPORT_BINDING_WRAPPED_2(create_copy_stencil, make_copy_stencil);

    using stencil_t = decltype(make_copy_stencil(std::declval<data_store_t>(), std::declval<data_store_t>()));

    BINDGEN_EXPORT_BINDING_WITH_SIGNATURE_WRAPPED_1(run_stencil, void(stencil_t const &), [](auto &&f) { f(); });
} // namespace
