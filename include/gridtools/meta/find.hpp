/*
 * GridTools Libraries
 * Copyright (c) 2019, ETH Zurich
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <type_traits>

namespace gridtools {
    namespace meta {
        constexpr size_t find_impl_helper(bool const *first, bool const *last) {
            return first == last || *first ? 0 : 1 + find_impl_helper(first + 1, last);
        }

        template <class, class>
        struct find_impl;

        template <template <class...> class L, class... Ts, class Key>
        struct find_impl<L<Ts...>, Key> {
            static constexpr bool values[] = {std::is_same<Ts, Key>::value...};
            static constexpr size_t value = find_impl_helper(values, values + sizeof...(Ts));
        };

        template <class List, class Key>
        struct find : std::integral_constant<size_t, find_impl<List, Key>::value> {};
    } // namespace meta
} // namespace gridtools
