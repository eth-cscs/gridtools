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

#include <utility>

#include <gtest/gtest.h>

#include "../common/array.hpp"
#include "../common/defs.hpp"
#include "../common/halo_descriptor.hpp"
#include "../common/selector.hpp"
#include "../stencil_composition/frontend.hpp"
#include "../storage/builder.hpp"
#include "../storage/sid.hpp"
#include "backend_select.hpp"
#include "verifier.hpp"

namespace gridtools {

    /*
    namespace _impl {
        template <class>
        struct default_layout;
        template <>
        struct default_layout<backend::cuda> {
            using type = layout_map<3, 2, 1, 0>;
        };
        template <>
        struct default_layout<backend::x86> {
            using type = layout_map<0, 3, 1, 2>;
        };
        template <>
        struct default_layout<backend::naive> {
            using type = layout_map<0, 1, 2, 3>;
        };

        template <std::size_t N, class DimSelector>
        using shorten_selector = meta::list_to_iseq<meta::take_c<N, meta::iseq_to_list<DimSelector>>>;

        template <class Backend,
                class Selector,
                class Layout = typename default_layout<Backend>::type,
                class Selector4D = shorten_selector<4, Selector>,
                class FilteredLayout = typename get_special_layout<Layout, Selector4D>::type>
        using select_layout = typename std::conditional_t<(Selector::size() > 4),
                extend_layout_map<FilteredLayout, Selector::size() - 4>,
                meta::lazy::id<FilteredLayout>>::type;
    } // namespace _impl

    template <class Backend, class Location, class Halo, class Selector>
    using icosahedral_storage_info_type = typename storage_traits<Backend>::template custom_layout_storage_info_t<
            impl::compute_uuid<Location::value, Selector>::value,
            _impl::select_layout<Backend, Selector>,
            Halo>;
*/

    template <size_t HaloSize = 0, class Axis = axis<1>>
    class computation_fixture : virtual public ::testing::Test {
        uint_t m_d1;
        uint_t m_d2;
        uint_t m_d3;

        template <class DataStorage>
        using halos_t = array<array<size_t, 2>, DataStorage::ndims>;

        template <class Storage>
        static halos_t<Storage> halos() {
            return {{{halo_size, halo_size}, {halo_size, halo_size}}};
        }

      public:
        static constexpr uint_t halo_size = HaloSize;

        halo_descriptor i_halo_descriptor() const {
            return {halo_size, halo_size, halo_size, m_d1 - halo_size - 1, m_d1};
        }
        halo_descriptor j_halo_descriptor() const {
            return {halo_size, halo_size, halo_size, m_d2 - halo_size - 1, m_d2};
        }

        auto make_grid() const { return ::gridtools::make_grid(i_halo_descriptor(), j_halo_descriptor(), Axis{m_d3}); }

        /// Fixture constructor takes the dimensions of the computation
        computation_fixture(uint_t d1, uint_t d2, uint_t d3) : m_d1(d1), m_d2(d2), m_d3(d3) {}

        uint_t d1() const { return m_d1; }
        uint_t d2() const { return m_d2; }
        uint_t d3() const { return m_d3; }

        uint_t &d1() { return m_d1; }
        uint_t &d2() { return m_d2; }
        uint_t &d3() { return m_d3; }

        template <class Expected, class Actual>
        void verify(
            Expected const &expected, Actual const &actual, double precision = default_precision<float_type>()) const {
            EXPECT_TRUE(
                gridtools::verify_data_store(expected, actual, halos<typename Actual::element_type>(), precision));
        }

#ifndef GT_ICOSAHEDRAL_GRIDS

        template <class T = float_type>
        auto builder() const {
            return storage::builder<storage_traits_t> //
                .dimensions(m_d1, m_d2, m_d3)         //
                .halos(HaloSize, HaloSize, 0)         //
                .template type<T>();
        }

        template <class T = float_type, class U, std::enable_if_t<!std::is_convertible<U const &, T>::value, int> = 0>
        auto make_storage(U const &arg) const {
            return builder<T>().initializer(arg).build();
        }

        template <class T = float_type, class U, std::enable_if_t<std::is_convertible<U const &, T>::value, int> = 0>
        auto make_storage(U const &arg) const {
            return builder<T>().value(arg).build();
        }

        template <class T = float_type>
        auto make_storage() const {
            return builder<T>().build();
        }

        using storage_type =
            decltype(storage::builder<storage_traits_t>.dimensions(0, 0, 0).template type<float_type>()());

        template <class T = float_type, class U>
        auto make_const_storage(U const &arg) const {
            return make_storage<T const>(arg);
        }
#else
        template <class Location, class T = float_type>
        auto builder() const {
            return storage::builder<storage_traits_t>          //
                .dimensions(m_d1, m_d2, m_d3, Location::value) //
                .halos(HaloSize, HaloSize, 0, 0)               //
                .template type<T>()                            //
                .template id<Location::value>();
        }

        template <class Location, class T = float_type>
        auto make_storage() const {
            return builder<Location, T>().build();
        }

        template <class Location,
            class T = float_type,
            class U,
            std::enable_if_t<!std::is_convertible<U const &, T>::value, int> = 0>
        auto make_storage(U const &arg) const {
            return builder<Location, T>().initializer(arg).build();
        }

        template <class Location,
            class T = float_type,
            class U,
            std::enable_if_t<std::is_convertible<U const &, T>::value, int> = 0>
        auto make_storage(U const &arg) const {
            return builder<Location, T>().value(arg).build();
        }

        /*

                template <class Location, class Selector = selector<1, 1, 1, 1>, class Halo = halo_t>
                using storage_info_t = icosahedral_storage_info_type<backend_t, Location, Halo, Selector>;

                template <class Location, class Selector = selector<1, 1, 1, 1>, class Halo = halo_t>
                using storage_type = storage_tr::data_store_t<float_type, storage_info_t<Location, Selector, Halo>>;

                using edge_2d_storage_type = storage_type<edges, selector<1, 1, 1, 0>>;
                using cell_2d_storage_type = storage_type<cells, selector<1, 1, 1, 0>>;
                using vertex_2d_storage_type = storage_type<vertices, selector<1, 1, 1, 0>>;

                template <class Location, class Selector = selector<1, 1, 1, 1, 1>>
                using storage_type_4d = storage_type<Location, Selector, halo<halo_size, 0, halo_size, 0, 0>>;

                template <uint_t I, class Location>
                using arg = gridtools::arg<I, void, Location>;

                template <uint_t I, class Location, typename Storage = float_type>
                using tmp_arg = gridtools::tmp_arg<I, Storage, Location>;

                template <class Location, class Storage = storage_type<Location>, class T = typename Storage::data_t>
                Storage make_storage(T &&obj = {}) const {
                    return {{m_d1, Location::n_colors::value, m_d2, m_d3}, std::forward<T>(obj)};
                }

                template <class Location, class Storage = storage_type<Location>>
                Storage make_storage(double val) const {
                    return {{m_d1, Location::n_colors::value, m_d2, m_d3}, (typename Storage::data_t)val};
                }

                template <class Location, class Storage = storage_type_4d<Location>, class T = typename Storage::data_t>
                Storage make_storage_4d(uint_t dim, T &&val = {}) {
                    return {{d1(), Location::n_colors::value, d2(), d3(), dim}, std::forward<T>(val)};
                }

                template <class Location, class Storage = storage_type_4d<Location>>
                Storage make_storage_4d(uint_t dim, double val) {
                    return {{d1(), Location::n_colors::value, d2(), d3(), dim}, (typename Storage::data_t)val};
                }
        */
#endif
    };
} // namespace gridtools
