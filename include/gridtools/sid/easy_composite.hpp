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

/**
 *  The clone of the sid composite that does not do strides compression.
 *  The implementation doesn't depend on stride kinds.
 */

#include <cassert>
#include <type_traits>
#include <utility>

#include "../common/defs.hpp"
#include "../common/for_each.hpp"
#include "../common/host_device.hpp"
#include "../common/hymap.hpp"
#include "../common/tuple.hpp"
#include "../common/tuple_util.hpp"
#include "../common/utility.hpp"
#include "../meta.hpp"
#include "concept.hpp"

namespace gridtools {
    namespace sid {
        namespace easy_composite {
            namespace impl_ {
                template <class ObjTup, class StrideTup, class Offset>
                GT_FUNCTION void composite_shift_impl(ObjTup &obj_tup, StrideTup &&stride_tup, Offset offset) {
                    tuple_util::host_device::for_each(
                        [offset](auto &obj, auto &&stride)
                            GT_FORCE_INLINE_LAMBDA { shift(obj, wstd::forward<decltype(stride)>(stride), offset); },
                        obj_tup,
                        wstd::forward<StrideTup>(stride_tup));
                }

                template <class Key, class Strides, class I = meta::st_position<get_keys<Strides>, Key>>
                using normalized_stride_type =
                    typename std::conditional_t<(I::value < tuple_util::size<Strides>::value),
                        tuple_util::lazy::element<I::value, Strides>,
                        meta::lazy::id<default_stride>>::type;

                template <class Keys>
                struct normalize_strides_f;

                template <template <class...> class L, class... Keys>
                struct normalize_strides_f<L<Keys...>> {
                    template <class Sid, class Strides = strides_type<Sid>>
                    tuple<normalized_stride_type<Keys, std::decay_t<Strides>>...> operator()(Sid const &sid) const {
                        return {get_stride<Keys>(get_strides(sid))...};
                    }
                };

                struct sum {
                    template <class Lhs, class Rhs>
                    GT_FUNCTION GT_CONSTEXPR auto operator()(Lhs &&lhs, Rhs &&rhs) const {
                        return wstd::forward<Lhs>(lhs) + wstd::forward<Rhs>(rhs);
                    }
                };
            } // namespace impl_

            template <class... Keys>
            struct keys {

                template <class... Ptrs>
                struct composite_ptr {
                    static_assert(sizeof...(Keys) == sizeof...(Ptrs), GT_INTERNAL_ERROR);

                    tuple<Ptrs...> m_vals;
                    GT_TUPLE_UTIL_FORWARD_GETTER_TO_MEMBER(composite_ptr, m_vals);
                    GT_TUPLE_UTIL_FORWARD_CTORS_TO_MEMBER(composite_ptr, m_vals);
                    GT_CONSTEXPR GT_FUNCTION decltype(auto) operator*() const {
                        return tuple_util::host_device::convert_to<hymap::keys<Keys...>::template values>(
                            tuple_util::host_device::transform([](auto const &ptr)
// Workaround for GCC 9 bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=90333
// The failure is observed with 9.3 as well even though they say it was already fixed there.
// gcc 10.1, 10.2 fails here as well. Disabling for all gcc 9 and 10 versions...
#if defined(__clang__) || !defined(__GNUC__) || (__GNUC__ != 9 && __GNUC__ != 10)
                                                                   GT_FORCE_INLINE_LAMBDA
#endif
                                -> decltype(auto) { return *ptr; },
                                m_vals));
                    }

                    friend keys hymap_get_keys(composite_ptr const &) { return {}; }
                };

                template <class... PtrHolders>
                struct composite_ptr_holder {
                    static_assert(sizeof...(Keys) == sizeof...(PtrHolders), GT_INTERNAL_ERROR);

                    tuple<PtrHolders...> m_vals;
                    GT_TUPLE_UTIL_FORWARD_GETTER_TO_MEMBER(composite_ptr_holder, m_vals);
                    GT_TUPLE_UTIL_FORWARD_CTORS_TO_MEMBER(composite_ptr_holder, m_vals);

                    GT_CONSTEXPR GT_FUNCTION auto operator()() const {
                        return tuple_util::host_device::convert_to<composite_ptr>(tuple_util::host_device::transform(
                            [](auto const &obj) GT_FORCE_INLINE_LAMBDA { return obj(); }, m_vals));
                    }

                    friend keys hymap_get_keys(composite_ptr_holder const &) { return {}; }
                };

                template <class... Ts>
                struct composite_entity {
                    static_assert(sizeof...(Keys) == sizeof...(Ts), GT_INTERNAL_ERROR);
                    tuple<Ts...> m_vals;
                    GT_TUPLE_UTIL_FORWARD_GETTER_TO_MEMBER(composite_entity, m_vals);
                    GT_TUPLE_UTIL_FORWARD_CTORS_TO_MEMBER(composite_entity, m_vals);
                    friend keys hymap_get_keys(composite_entity const &) { return {}; }

                    template <class... Ptrs>
                    friend GT_CONSTEXPR GT_FUNCTION composite_ptr<Ptrs...> operator+(
                        composite_ptr<Ptrs...> const &lhs, composite_entity const &rhs) {
                        return tuple_util::host_device::transform(impl_::sum(), lhs, rhs);
                    }

                    template <class... PtrHolders>
                    friend composite_ptr_holder<PtrHolders...> operator+(
                        composite_ptr_holder<PtrHolders...> const &lhs, composite_entity const &rhs) {
                        return tuple_util::transform(impl_::sum(), lhs, rhs);
                    }

                    template <class... Ptrs, class Offset>
                    friend GT_FUNCTION void sid_shift(
                        composite_ptr<Ptrs...> &ptr, composite_entity const &stride, Offset offset) {
                        impl_::composite_shift_impl(ptr.m_vals, stride.m_vals, offset);
                    }

                    template <class... Ptrs, class Offset>
                    friend GT_FUNCTION void sid_shift(
                        composite_ptr<Ptrs...> &ptr, composite_entity &&stride, Offset offset) {
                        impl_::composite_shift_impl(ptr.m_vals, wstd::move(stride.m_vals), offset);
                    }
                };

                template <class... PtrDiffs, class... Strides, class Offset>
                friend GT_FUNCTION void sid_shift(composite_entity<PtrDiffs...> &ptr_diff,
                    composite_entity<Strides...> const &stride,
                    Offset offset) {
                    impl_::composite_shift_impl(ptr_diff.m_vals, stride.m_vals, offset);
                }

                template <class... PtrDiffs, class... Strides, class Offset>
                friend GT_FUNCTION void sid_shift(
                    composite_entity<PtrDiffs...> &ptr_diff, composite_entity<Strides...> &&stride, Offset offset) {
                    impl_::composite_shift_impl(ptr_diff.m_vals, wstd::move(stride.m_vals), offset);
                }

                struct convert_f {
                    template <template <class...> class L, class... Ts>
                    composite_entity<std::remove_reference_t<Ts>...> operator()(L<Ts...> &&tup) const {
                        return {std::move(tup)};
                    }
                };

                template <class... Sids>
                struct values {
                    static_assert(sizeof...(Keys) == sizeof...(Sids), GT_INTERNAL_ERROR);
#if defined(__CUDACC_VER_MAJOR__) && __CUDACC_VER_MAJOR__ == 11 && __CUDACC_VER_MINOR__ < 1
#else
                    static_assert(conjunction<is_sid<Sids>...>::value, GT_INTERNAL_ERROR);
#endif

                    tuple<Sids...> m_sids;

                    using stride_keys_t = meta::dedup<meta::concat<get_keys<strides_type<Sids>>...>>;

                    using stride_hymap_keys_t = meta::rename<hymap::keys, stride_keys_t>;

                    template <class... Values>
                    using stride_hymap_ctor = typename stride_hymap_keys_t::template values<Values...>;

                    // A helper for generating strides_t
                    // It is a meta function from the stride key to the stride type
                    template <class Key>
                    using get_stride_type =
                        composite_entity<impl_::normalized_stride_type<Key, std::decay_t<strides_type<Sids>>>...>;

                    // all `SID` types are here
                    using ptr_holder_t = composite_ptr_holder<ptr_holder_type<Sids>...>;
                    using ptr_t = composite_ptr<ptr_type<Sids>...>;
                    using strides_t = meta::rename<stride_hymap_ctor, meta::transform<get_stride_type, stride_keys_t>>;
                    using ptr_diff_t = composite_entity<ptr_diff_type<Sids>...>;

                    // Here the `SID` concept is modeled

                    friend ptr_holder_t sid_get_origin(values &obj) {
                        return tuple_util::transform(
                            [](auto obj) GT_FORCE_INLINE_LAMBDA { return get_origin(obj); }, obj.m_sids);
                    }

                    template <class U = strides_t, std::enable_if_t<!meta::is_empty<U>::value, int> = 0>
                    friend strides_t sid_get_strides(values const &obj) {
                        return tuple_util::transform(convert_f(),
                            tuple_util::transpose(
                                tuple_util::transform(impl_::normalize_strides_f<stride_keys_t>(), obj.m_sids)));
                    }

                    template <class U = strides_t, std::enable_if_t<meta::is_empty<U>::value, int> = 0>
                    friend strides_t sid_get_strides(values const &) {
                        return {};
                    }

                    friend ptr_diff_t sid_get_ptr_diff(values const &) { return {}; }

                    friend meta::list<strides_kind<Sids>...> sid_get_strides_kind(values const &) { return {}; }

                    // Here the `tuple_like` concept is modeled
                    struct getter {
                        template <size_t I>
                        static decltype(auto) get(values const &obj) noexcept {
                            return tuple_util::get<I>(obj.m_sids);
                        }
                        template <size_t I>
                        static decltype(auto) get(values &obj) noexcept {
                            return tuple_util::get<I>(obj.m_sids);
                        }
                        template <size_t I>
                        static decltype(auto) get(values &&obj) noexcept {
                            return tuple_util::get<I>(std::move(obj).m_sids);
                        }
                    };
                    friend getter tuple_getter(values const &) { return {}; }

                    template <class Arg,
                        class... Args,
                        std::enable_if_t<std::is_constructible<tuple<Sids...>, Arg &&, Args &&...>::value, int> = 0>
                    values(Arg &&arg, Args &&... args) noexcept
                        : m_sids(std::forward<Arg>(arg), std::forward<Args>(args)...) {}
                    values() = default;
                    values(values const &) = default;
                    values(values &&) = default;
                    values &operator=(values const &) = default;
                    values &operator=(values &&) = default;

                    // hymap concept
                    friend keys hymap_get_keys(values const &) { return {}; }
                };
            };

            template <class... Keys>
            struct make_f {
                template <class... Sids>
                constexpr auto operator()(Sids &&... sids) const {
                    return tuple_util::make<keys<Keys...>::template values>(std::forward<Sids>(sids)...);
                }
            };

            template <class... Keys>
            constexpr make_f<Keys...> make = {};
        } // namespace easy_composite
    }     // namespace sid
} // namespace gridtools
