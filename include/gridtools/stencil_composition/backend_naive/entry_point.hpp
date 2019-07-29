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

#include <memory>

#include "../../common/defs.hpp"
#include "../../common/functional.hpp"
#include "../../common/generic_metafunctions/for_each.hpp"
#include "../../common/hymap.hpp"
#include "../../common/integral_constant.hpp"
#include "../../common/tuple_util.hpp"
#include "../../meta.hpp"
#include "../dim.hpp"
#include "../sid/allocator.hpp"
#include "../sid/composite.hpp"
#include "../sid/concept.hpp"
#include "../sid/contiguous.hpp"
#include "../sid/loop.hpp"
#include "../sid/sid_shift_origin.hpp"
#include "../stage_matrix.hpp"

namespace gridtools {
    namespace naive {
        template <class Spec, class Grid, class DataStores>
        void gridtools_backend_entry_point(backend, Spec, Grid const &grid, DataStores data_stores) {
            auto alloc = sid::make_allocator(&std::make_unique<char[]>);
            using stages_t = stage_matrix::make_split_view<Spec>;
            using plh_map_t = typename stages_t::plh_map_t;
            using keys_t = meta::rename<sid::composite::keys, meta::transform<meta::first, plh_map_t>>;
            auto composite = tuple_util::convert_to<keys_t::template values>(tuple_util::transform(
                overload(
                    [&](std::true_type, auto info) {
                        auto extent = info.extent();
                        auto num_colors = info.num_colors();
                        auto offsets = tuple_util::make<hymap::keys<dim::i, dim::j>::values>(
                            -extent.minus(dim::i()), -extent.minus(dim::j()));
                        auto sizes = tuple_util::make<hymap::keys<dim::c, dim::k, dim::j, dim::i>::values>(
                            num_colors, grid.k_size(), grid.j_size(extent), grid.i_size(extent));
                        using stride_kind = meta::list<decltype(extent), decltype(num_colors)>;
                        return sid::shift_sid_origin(
                            sid::make_contiguous<decltype(info.data()), ptrdiff_t, stride_kind>(alloc, sizes), offsets);
                    },
                    [&](std::false_type, auto info) { return at_key<decltype(info.plh())>(data_stores); }),
                meta::transform<stage_matrix::get_is_tmp, plh_map_t>(),
                plh_map_t()));
            auto origin = sid::get_origin(composite);
            auto strides = sid::get_strides(composite);
            for_each<stages_t>([&](auto stage) {
                tuple_util::for_each(
                    [&](auto cell) {
                        auto ptr = origin();
                        auto extent = cell.extent();
                        auto interval = cell.interval();
                        sid::shift(ptr, sid::get_stride<dim::i>(strides), extent.minus(dim::i()));
                        sid::shift(ptr, sid::get_stride<dim::j>(strides), extent.minus(dim::j()));
                        sid::shift(ptr, sid::get_stride<dim::k>(strides), grid.k_start(interval, cell.execution()));
                        auto i_loop = sid::make_loop<dim::i>(grid.i_size(extent));
                        auto j_loop = sid::make_loop<dim::j>(grid.j_size(extent));
                        auto k_loop = sid::make_loop<dim::k>(grid.k_size(interval), cell.k_step());
                        i_loop(j_loop(k_loop(cell)))(ptr, strides);
                    },
                    stage.cells());
            });
        }
    } // namespace naive
} // namespace gridtools
