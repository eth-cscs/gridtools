/*
 * GridTools Libraries
 * Copyright (c) 2019, ETH Zurich
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <type_traits>

#include "../defs.hpp"
#include "../host_device.hpp"

namespace gridtools {
    /** \ingroup common
        @{
        \ingroup allmeta
        @{
        \ingroup variadic
        @{
    */

    /**
     * metafunction is to simply "store" a variadic pack. A typical use case is when we need to typedef a variadic pack
     * template<typename ... Args>
     * struct a { typedef variadic_typedef<Args...> type; }
     */
    template <typename... Args>
    struct variadic_typedef;

    namespace impl_ {

        template <ushort_t Idx, typename First, typename... Args>
        struct get_elem {
            GT_STATIC_ASSERT((Idx <= sizeof...(Args)), "Out of bound access in variadic pack");
            typedef typename ::gridtools::variadic_typedef<Args...>::template get_elem<Idx - 1>::type type;
        };

        template <typename First, typename... Args>
        struct get_elem<0, First, Args...> {
            typedef First type;
        };
    } // namespace impl_

    /// \private
    template <typename First, typename... Args>
    struct variadic_typedef<First, Args...> {

        // metafunction that returns a type of a variadic pack by index
        template <ushort_t Idx>
        struct get_elem {
            GT_STATIC_ASSERT((Idx <= sizeof...(Args)), "Out of bound access in variadic pack");
            typedef typename impl_::template get_elem<Idx, First, Args...>::type type;
        };

        template <typename Elem>
        static constexpr int_t find(const ushort_t pos = 0) {
            return std::is_same<First, Elem>::value ? pos : variadic_typedef<Args...>::template find<Elem>(pos + 1);
        }

        static constexpr ushort_t length = sizeof...(Args) + 1;
    };

    /// \private
    template <>
    struct variadic_typedef<> {

        // metafunction that returns a type of a variadic pack by index
        template <ushort_t Idx>
        struct get_elem {};

        template <typename Elem>
        static constexpr int_t find(const ushort_t pos = 0) {
            return -1;
        }

        static constexpr ushort_t length = 0;
    };

    /**
     * helper functor that returns a particular argument of a variadic pack by index
     * @tparam Idx index of the variadic pack argument to be returned
     */
    template <int Idx>
    struct get_from_variadic_pack {
        template <typename First, typename... Accessors>
        GT_FUNCTION static constexpr typename variadic_typedef<First, Accessors...>::template get_elem<Idx>::type apply(
            First first, Accessors... args) {
            GT_STATIC_ASSERT((Idx <= sizeof...(Accessors)), "Out of bound access in variadic pack");

            return get_from_variadic_pack<Idx - 1>::apply(args...);
        }
    };

    /// \private
    template <>
    struct get_from_variadic_pack<0> {
        template <typename First, typename... Accessors>
        GT_FUNCTION static constexpr First apply(First first, Accessors... args) {
            return first;
        }
    };
    /** @} */
    /** @} */
    /** @} */

} // namespace gridtools
