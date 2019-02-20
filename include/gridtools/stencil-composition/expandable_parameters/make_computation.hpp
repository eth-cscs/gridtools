/*
 * GridTools
 *
 * Copyright (c) 2019, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "../make_computation.hpp"

#include "expand_factor.hpp"
#include "intermediate_expand.hpp"

namespace gridtools {
    namespace _impl {

        template <uint_t Factor, bool IsStateful, class Backend>
        struct make_intermediate_expand_f {
            template <class Grid,
                class... Args,
                class ArgsPair = decltype(split_args<is_arg_storage_pair>(std::forward<Args>(std::declval<Args>())...)),
                class ArgStoragePairs = GT_META_CALL(decay_elements, typename ArgsPair::first_type),
                class Msses = GT_META_CALL(decay_elements, typename ArgsPair::second_type)>
            intermediate_expand<Factor, IsStateful, Backend, Grid, ArgStoragePairs, Msses> operator()(
                Grid const &grid, Args &&... args) const {
                // split arg_storage_pair and mss descriptor arguments and forward it to intermediate constructor
                auto &&args_pair = split_args<is_arg_storage_pair>(std::forward<Args>(args)...);
                return {grid, std::move(args_pair.first), std::move(args_pair.second)};
            }
        };

    } // namespace _impl

#ifndef NDEBUG
#define GT_POSITIONAL_WHEN_DEBUGGING true
#else
#define GT_POSITIONAL_WHEN_DEBUGGING false
#endif

    /// generator for intermediate/intermediate_expand
    ///
    template <class Backend, size_t N, class... GridArgs, class Arg, class... Args>
    auto make_expandable_computation(expand_factor<N>, grid<GridArgs...> const &grid, Arg &&arg, Args &&... args)
        GT_AUTO_RETURN((_impl::make_intermediate_expand_f<N, GT_POSITIONAL_WHEN_DEBUGGING, Backend>{}(
            grid, std::forward<Arg>(arg), std::forward<Args>(args)...)));

#undef GT_POSITIONAL_WHEN_DEBUGGING

    template <class Backend, size_t N, class... GridArgs, class Arg, class... Args>
    auto make_expandable_positional_computation(
        expand_factor<N>, grid<GridArgs...> const &grid, Arg &&arg, Args &&... args)
        GT_AUTO_RETURN((_impl::make_intermediate_expand_f<N, true, Backend>{}(
            grid, std::forward<Arg>(arg), std::forward<Args>(args)...)));
} // namespace gridtools
