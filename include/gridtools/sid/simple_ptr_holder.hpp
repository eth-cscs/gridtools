/*
 * GridTools
 *
 * Copyright (c) 2014-2023, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef GT_TARGET_ITERATING
// DON'T USE #pragma once HERE!!!
#ifndef GT_SID_SIMPLE_PTR_HOLDER_HPP_
#define GT_SID_SIMPLE_PTR_HOLDER_HPP_

#include <utility>

#include "../common/defs.hpp"
#include "../common/host_device.hpp"
#include "../common/ldg_ptr.hpp"

#define GT_FILENAME <gridtools/sid/simple_ptr_holder.hpp>
#include GT_ITERATE_ON_TARGETS()
#undef GT_FILENAME

#endif
#else

namespace gridtools {
    namespace sid {
        GT_TARGET_NAMESPACE {
            template <class T>
            struct simple_ptr_holder {
                T m_val;

#if not GT_HAS_AGGREGATE_CTAD
                // Enables CTAD in C++17.
                simple_ptr_holder() = default;
                GT_TARGET GT_FORCE_INLINE constexpr simple_ptr_holder(T const &ptr) : m_val{ptr} {}
#endif
                GT_TARGET GT_FORCE_INLINE constexpr decltype(auto) operator()() const { return as_ldg_ptr(m_val); }
            };

            template <class T>
            [[deprecated("use simple_ptr_holder class template argument deduction")]] constexpr simple_ptr_holder<T>
            make_simple_ptr_holder(T const &ptr) {
                return {ptr};
            }

            template <class T, class Arg>
            constexpr auto operator+(simple_ptr_holder<T> const &obj, Arg &&arg) {
                return simple_ptr_holder(obj.m_val + std::forward<Arg>(arg));
            }

            template <class T, class Arg>
            constexpr auto operator+(simple_ptr_holder<T> &&obj, Arg &&arg) {
                return simple_ptr_holder(std::move(obj).m_val + std::forward<Arg>(arg));
            }
        }
    } // namespace sid
} // namespace gridtools

#endif
