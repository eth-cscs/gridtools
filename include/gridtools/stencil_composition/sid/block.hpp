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

#include "../../common/generic_metafunctions/for_each.hpp"
#include "../../common/host_device.hpp"
#include "../../common/hymap.hpp"
#include "../../common/tuple_util.hpp"
#include "../../meta.hpp"
#include "./concept.hpp"
#include "./delegate.hpp"

namespace gridtools {
    namespace sid {
        template <class Dim>
        struct blocked_dim {
            using type = blocked_dim;
        };

        namespace block_impl_ {
            template <class Stride, class BlockSize>
            struct blocked_stride {
                Stride m_stride;
                BlockSize m_block_size;
            };

            template <class Ptr, class Stride, class BlockSize, class Offset>
            GT_FUNCTION auto sid_shift(Ptr &ptr, blocked_stride<Stride, BlockSize> const &stride, Offset const &offset)
                GT_AUTO_RETURN(shift(ptr, stride.m_stride, stride.m_block_size *offset));

            template <class Stride>
            using just_multiply =
                bool_constant<std::is_integral<Stride>::value || concept_impl_::is_integral_constant<Stride>::value>;

            template <class Stride, class BlockSize, enable_if_t<!just_multiply<Stride>::value, int> = 0>
            blocked_stride<Stride, BlockSize> block_stride(Stride const &stride, BlockSize const &block_size) {
                return {stride, block_size};
            }

            template <class Stride, class BlockSize, enable_if_t<just_multiply<Stride>::value, int> = 0>
            Stride block_stride(Stride const &stride, BlockSize const &block_size) {
                return stride * block_size;
            }

            template <class Stride, class BlockSize>
            using blocked_stride_type = decltype(block_stride(std::declval<Stride>(), std::declval<BlockSize>()));

            template <class Dim, class BlockedDim>
            struct generate_strides_f;

            template <class Dim>
            struct generate_strides_f<Dim, Dim> {
                using type = generate_strides_f;

                template <class Strides, class BlockMap>
                auto operator()(Strides const &strides, BlockMap const &) GT_AUTO_RETURN(at_key<Dim>(strides));
            };

            template <class Dim>
            struct generate_strides_f<Dim, blocked_dim<Dim>> {
                using type = generate_strides_f;

                template <class Strides, class BlockMap>
                auto operator()(Strides const &strides, BlockMap const &block_map) const
                    GT_AUTO_RETURN(block_stride(at_key<Dim>(strides), at_key<Dim>(block_map)));
            };

            template <class Sid, class BlockMap>
            class blocked_sid : public delegate<Sid> {
                BlockMap m_block_map;

                using strides_map_t = GT_META_CALL(hymap::to_meta_map, GT_META_CALL(strides_type, Sid));
                using strides_dims_t = GT_META_CALL(meta::transform, (meta::first, strides_map_t));

                template <class MapEntry, class Dim = GT_META_CALL(meta::first, MapEntry)>
                GT_META_DEFINE_ALIAS(is_strides_dim, meta::st_contains, (strides_dims_t, Dim));

                using block_map_t = GT_META_CALL(
                    meta::filter, (is_strides_dim, GT_META_CALL(hymap::to_meta_map, BlockMap)));
                using block_dims_t = GT_META_CALL(meta::transform, (meta::first, block_map_t));

                template <class MapEntry,
                    class Dim = GT_META_CALL(meta::first, MapEntry),
                    class BlockSize = GT_META_CALL(meta::second, MapEntry),
                    class Stride = GT_META_CALL(meta::second, (GT_META_CALL(meta::mp_find, (strides_map_t, Dim))))>
                GT_META_DEFINE_ALIAS(block, meta::list, (blocked_dim<Dim>, blocked_stride_type<Stride, BlockSize>));

                using blocked_strides_map_t = GT_META_CALL(meta::transform, (block, block_map_t));

                using strides_t = GT_META_CALL(hymap::from_meta_map,
                    (GT_META_CALL(meta::concat, (strides_map_t, GT_META_CALL(meta::transform, (block, block_map_t))))));

                using original_dims_t = GT_META_CALL(meta::concat, (strides_dims_t, block_dims_t));
                using blocked_dims_t = GT_META_CALL(get_keys, strides_t);

                template <class L>
                GT_META_DEFINE_ALIAS(make_generator, meta::rename, (generate_strides_f, L));
                using generators_t = GT_META_CALL(
                    meta::transform, (make_generator, GT_META_CALL(meta::zip, (original_dims_t, blocked_dims_t))));

              public:
                blocked_sid(Sid const &impl, BlockMap const &block_map) noexcept
                    : delegate<Sid>(impl), m_block_map(block_map) {}

                friend strides_t sid_get_strides(blocked_sid const &obj) {
                    return tuple_util::host_device::generate<generators_t, strides_t>(
                        get_strides(obj.impl()), obj.m_block_map);
                }
            };

        } // namespace block_impl_

        template <class Sid, class BlockMap>
        block_impl_::blocked_sid<Sid, BlockMap> block(Sid const &sid, BlockMap const &block_map) {
            return {sid, block_map};
        }
    } // namespace sid
} // namespace gridtools
