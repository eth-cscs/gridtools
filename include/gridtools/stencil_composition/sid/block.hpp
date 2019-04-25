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
#include "../../meta/concat.hpp"
#include "../../meta/push_back.hpp"
#include "../../meta/rename.hpp"
#include "../../meta/st_contains.hpp"
#include "./concept.hpp"
#include "./synthetic.hpp"

namespace gridtools {
    namespace sid {
        template <class Dim>
        struct blocked_dim {
            using type = blocked_dim<Dim>;
        };

        namespace block_impl_ {
            template <class Stride, class BlockSize>
            struct blocked_stride {
                Stride m_stride;
                BlockSize m_block_size;
            };

            template <class Stride, class BlockSize>
            blocked_stride<Stride, BlockSize> make_blocked_stride(Stride const &stride, BlockSize const &block_size) {
                return {stride, block_size};
            }

            template <class Ptr, class Stride, class BlockSize, class Offset>
            GT_FUNCTION auto sid_shift(Ptr &ptr, blocked_stride<Stride, BlockSize> const &stride, Offset const &offset)
                GT_AUTO_RETURN(shift(ptr, stride.m_stride, stride.m_block_size *offset));

            template <class Strides, class BlockMap>
            struct block_strides_f {
                Strides m_strides;
                BlockMap m_map;

                template <class Dim>
                auto operator()(blocked_dim<Dim>) const
                    GT_AUTO_RETURN(make_blocked_stride(get_stride<Dim>(m_strides), at_key<Dim>(m_map)));

                template <class Dim>
                auto operator()(Dim) const GT_AUTO_RETURN(get_stride<Dim>(m_strides));
            };

            template <class Strides, class BlockMap>
            using blocked_strides_keys = GT_META_CALL(meta::concat,
                (GT_META_CALL(get_keys, Strides),
                    GT_META_CALL(meta::transform, (blocked_dim, GT_META_CALL(get_keys, BlockMap)))));

            template <class Strides,
                class BlockMap,
                class Keys = blocked_strides_keys<Strides, BlockMap>,
                class KeysToKeys = GT_META_CALL(meta::rename, (Keys::template values, Keys))>
            auto block_strides(Strides const &strides, BlockMap const &block_map)
                GT_AUTO_RETURN(tuple_util::host_device::transform(
                    block_strides_f<Strides, BlockMap>{strides, block_map}, KeysToKeys{}));
        } // namespace block_impl_

        template <class Sid, class BlockMap>
        auto block(Sid const &s, BlockMap const &block_map)
            GT_AUTO_RETURN((synthetic()
                                .set<property::origin>(get_origin(s))
                                .template set<property::strides>(block_impl_::block_strides(get_strides(s), block_map))
                                .template set<property::ptr_diff, ptr_diff_type<Sid>>()
                                .template set<property::strides_kind, strides_kind<Sid>>()));
    } // namespace sid
} // namespace gridtools
