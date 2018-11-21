/*
  GridTools Libraries

  Copyright (c) 2017, ETH Zurich and MeteoSwiss
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  For information: http://eth-cscs.github.io/gridtools/
*/
#pragma once

#include <tuple>
#include <type_traits>

#include "../../common/defs.hpp"
#include "../../common/generic_metafunctions/meta.hpp"
#include "../../common/generic_metafunctions/type_traits.hpp"
#include "../../common/host_device.hpp"
#include "../../common/tuple_util.hpp"

/**
 *   Basic API for Stencil Iterable Data (aka SID) concept.
 */

namespace gridtools {
    namespace sid {
        namespace impl_ {

            /**
             *  generic trait for integral constant
             *
             *  TODO(anstaf) : consider moving it to `meta` or to `common`
             */
            template <class T, class = void>
            struct is_integral_constant : std::false_type {};
            template <class T>
            struct is_integral_constant<T,
                enable_if_t<std::is_empty<T>::value && std::is_integral<decltype(T::value)>::value>> : std::true_type {
            };

            /**
             *  generic trait for integral constant of Val
             *
             *  TODO(anstaf) : consider moving it to `meta` or to `common`
             */
            template <int Val, class T, class = void>
            struct is_integral_constant_of : std::false_type {};

            template <int Val, class T>
            struct is_integral_constant_of<Val, T, enable_if_t<std::is_empty<T>::value &&int(T::value) == Val>>
                : std::true_type {};

            // BEGIN `get_origin` PART

            // `sid_get_origin` doesn't have fallback

            /**
             *  `Ptr` type is deduced from `sid_get_origin`
             */
            template <class Sid>
            GT_META_DEFINE_ALIAS(ptr_type, meta::id, decltype(sid_get_origin(std::declval<Sid const &>())));

            /**
             *  `Reference` type is deduced from `Ptr` type
             */
            template <class Sid, class Ptr = GT_META_CALL(ptr_type, Sid)>
            GT_META_DEFINE_ALIAS(reference_type, meta::id, decltype(*std::declval<Ptr const &>()));

            /**
             *  `PtrDiff` type is deduced from `Ptr` type
             */
            template <class Sid, class Ptr = GT_META_CALL(ptr_type, Sid)>
            GT_META_DEFINE_ALIAS(
                ptr_diff_type, meta::id, decltype(std::declval<Ptr const &>() - std::declval<Ptr const &>()));

            /**
             *  `get_origin` delegates to `sid_get_origin`
             */
            template <class Sid>
            constexpr GT_FUNCTION GT_META_CALL(ptr_type, Sid) get_origin(Sid const &obj) {
                return sid_get_origin(obj);
            }

            // END `get_origin` PART

            // BEGIN `get_strides` PART

            /**
             *  `sid_get_strides` fallback
             *
             *  By default Sid doesn't provides its own strides.
             */
            template <class Sid>
            constexpr GT_FUNCTION std::tuple<> sid_get_strides(Sid const &) {
                return {};
            }

            /**
             *  `Strides` type is deduced from `sid_get_strides`
             */
            template <class Sid>
            GT_META_DEFINE_ALIAS(strides_type, meta::id, decltype(sid_get_strides(std::declval<Sid const &>())));

            /**
             *  `get_strides` delegates to `sid_get_strides`
             */
            template <class Sid>
            constexpr GT_FUNCTION GT_META_CALL(strides_type, Sid) get_strides(Sid const &obj) {
                return sid_get_strides(obj);
            }

            // END `get_strides` PART

            // BEGIN `strides_kind` PART

            /**
             *  `sid_get_strides_kind` fallback
             *
             *  It is enabled only if `Strides` type has no members.
             */
            template <class Sid, class Strides = GT_META_CALL(strides_type, Sid)>
            enable_if_t<std::is_empty<Strides>::value, Strides> sid_get_strides_kind(Sid const &);

            /**
             *  `strides_kind` is deduced from `sid_get_strides_kind`
             */
            template <class Sid>
            GT_META_DEFINE_ALIAS(strides_kind, meta::id, decltype(sid_get_strides_kind(std::declval<Sid const &>())));

            // END `strides_kind` PART

            // BEGIN `bounds_validator` PART

            /**
             *  optimistic validator
             */
            struct always_happy {
                constexpr GT_FUNCTION bool operator()(...) const { return true; }
            };

            /**
             *  `sid_get_bounds_validator` fallback
             *
             *  By default don't validate.
             */
            template <class Sid>
            constexpr GT_FUNCTION always_happy sid_get_bounds_validator(Sid const &) {
                return {};
            }

            /**
             *  `BoundsValidator` types is deduced from `sid_get_bounds_validator`
             */
            template <class Sid>
            GT_META_DEFINE_ALIAS(
                bounds_validator_type, meta::id, decltype(sid_get_bounds_validator(std::declval<Sid const &>())));

            /**
             *  `get_bounds_validator` delegates to `sid_get_bounds_validator`
             */
            template <class Sid>
            constexpr GT_FUNCTION GT_META_CALL(bounds_validator_type, Sid) get_bounds_validator(Sid const &obj) {
                return sid_get_bounds_validator(obj);
            }

            // END `bounds_validator` PART

            // BEGIN `bounds_validator_kind` PART

            /**
             *  `sid_get_bounds_validator_kind` fallback
             *
             *  It is enabled only if `BoundsValidator` type has no members.
             */
            template <class Sid, class BoundsValidator = GT_META_CALL(bounds_validator_type, Sid)>
            enable_if_t<std::is_empty<BoundsValidator>::value, BoundsValidator> sid_get_bounds_validator_kind(
                Sid const &);

            /**
             *  `bounds_validator_kind` is deduced from `sid_get_bounds_validator_kind`
             */
            template <class Sid>
            GT_META_DEFINE_ALIAS(
                bounds_validator_kind, meta::id, decltype(sid_get_bounds_validator_kind(std::declval<Sid const &>())));

            // END `bounds_validator_kind` PART

            // BEGIN `shift` PART

            // no fallback for `sid_shift`

            /**
             *  Predicate that determines if `shift` needs to be apply
             *
             *  If stride of offset are zero or the target has no state, we don't need to shift
             */
            template <class T, class Stride, class Offset>
            GT_META_DEFINE_ALIAS(need_shift,
                bool_constant,
                (!(std::is_empty<T>::value || is_integral_constant_of<0, Stride>::value ||
                    is_integral_constant_of<0, Offset>::value)));

            /**
             *  true if we can do implement shift as `obj += stride * offset`
             */
            template <class T, class Strides, class = void>
            struct is_default_shiftable : std::false_type {};
            template <class T, class Stride>
            struct is_default_shiftable<T,
                Stride,
                enable_if_t<std::is_void<decltype(
                    void(std::declval<T &>() += std::declval<Stride const &>() * std::declval<int_t>()))>::value>>
                : std::true_type {};

            /**
             *  True if T has operator++
             */
            template <class T, class = void>
            struct has_inc : std::false_type {};
            template <class T>
            struct has_inc<T, enable_if_t<std::is_void<decltype(void(++std::declval<T &>()))>::value>>
                : std::true_type {};

            /**
             *  True if T has operator--
             */
            template <class T, class = void>
            struct has_dec : std::false_type {};
            template <class T>
            struct has_dec<T, enable_if_t<std::is_void<decltype(void(--std::declval<T &>()))>::value>>
                : std::true_type {};

            /**
             *  True if T has operator-=
             */
            template <class T, class Arg, class = void>
            struct has_dec_assignment : std::false_type {};
            template <class T, class Arg>
            struct has_dec_assignment<T,
                Arg,
                enable_if_t<std::is_void<decltype(void(std::declval<T &>() -= std::declval<Arg const &>()))>::value>>
                : std::true_type {};

            /**
             *  noop `shift` overload
             */
            template <class T, class Stride, class Offset>
            GT_FUNCTION enable_if_t<!need_shift<T, Stride, Offset>::value> shift(T &, Stride const &stride, Offset) {}

            /**
             * `shift` overload that delegates to `sid_shift`
             *
             *  Enabled only if shift can not be implemented as `obj += stride * offset`
             */
            template <class T, class Stride, class Offset>
            GT_FUNCTION enable_if_t<need_shift<T, Stride, Offset>::value && !is_default_shiftable<T, Stride>::value>
            shift(T &obj, Stride const &stride, Offset offset) {
                sid_shift(obj, stride, offset);
            }

            /**
             *  `shift` overload where both `stride` and `offset` are known in compile time
             */
            template <class T, class Stride, class Offset, int_t PtrOffset = Stride::value *Offset::value>
            GT_FUNCTION enable_if_t<need_shift<T, Stride, Offset>::value && is_default_shiftable<T, Stride>::value &&
                                    !(has_inc<T>::value && PtrOffset == 1) && !(has_dec<T>::value && PtrOffset == -1)>
            shift(T &obj, Stride const &, Offset) {
                obj += std::integral_constant<int_t, PtrOffset>{};
            }

            /**
             *  `shift` overload where the stride and offset are both 1 (or both -1)
             */
            template <class T, class Stride, class Offset, int_t PtrOffset = Stride::value *Offset::value>
            GT_FUNCTION enable_if_t<need_shift<T, Stride, Offset>::value && is_default_shiftable<T, Stride>::value &&
                                    has_inc<T>::value && PtrOffset == 1>
            shift(T &obj, Stride const &, Offset) {
                ++obj;
            }

            /**
             *  `shift` overload where the stride are offset are both 1, -1 (or -1, 1)
             */
            template <class T, class Stride, class Offset, int_t PtrOffset = Stride::value *Offset::value>
            GT_FUNCTION enable_if_t<need_shift<T, Stride, Offset>::value && is_default_shiftable<T, Stride>::value &&
                                    has_dec<T>::value && PtrOffset == -1>
            shift(T &obj, Stride const &, Offset) {
                --obj;
            }

            /**
             *  `shift` overload where the offset is 1
             */
            template <class T, class Stride, class Offset>
            GT_FUNCTION enable_if_t<need_shift<T, Stride, Offset>::value && is_default_shiftable<T, Stride>::value &&
                                    !is_integral_constant<Stride>::value && is_integral_constant_of<1, Offset>::value>
            shift(T &obj, Stride const &stride, Offset) {
                obj += stride;
            }

            /**
             *  `shift` overload where the offset is -1
             */
            template <class T, class Stride, class Offset>
            GT_FUNCTION enable_if_t<need_shift<T, Stride, Offset>::value && is_default_shiftable<T, Stride>::value &&
                                    !is_integral_constant<Stride>::value &&
                                    is_integral_constant_of<-1, Offset>::value && has_dec_assignment<T, Stride>::value>
            shift(T &obj, Stride const &stride, Offset) {
                obj -= stride;
            }

            /**
             *  `shift` overload where the stride is 1
             */
            template <class T, class Stride, class Offset>
            GT_FUNCTION enable_if_t<need_shift<T, Stride, Offset>::value && is_default_shiftable<T, Stride>::value &&
                                    is_integral_constant_of<1, Stride>::value && !is_integral_constant<Offset>::value>
            shift(T &obj, Stride const &, Offset offset) {
                obj += offset;
            }

            /**
             *  `shift` overload where the stride is -1
             */
            template <class T, class Stride, class Offset>
            GT_FUNCTION enable_if_t<need_shift<T, Stride, Offset>::value && is_default_shiftable<T, Stride>::value &&
                                    is_integral_constant_of<-1, Stride>::value &&
                                    !is_integral_constant<Offset>::value && has_dec_assignment<T, Stride>::value>
            shift(T &obj, Stride const &, Offset offset) {
                obj -= offset;
            }

            /**
             *  `shift` overload, default version
             */
            template <class T, class Stride, class Offset>
            GT_FUNCTION
                enable_if_t<need_shift<T, Stride, Offset>::value && is_default_shiftable<T, Stride>::value &&
                            !(is_integral_constant<Stride>::value && is_integral_constant<Offset>::value) &&
                            !(is_integral_constant_of<1, Stride>::value || is_integral_constant_of<1, Offset>::value) &&
                            !(has_dec_assignment<T, Stride>::value && (is_integral_constant_of<-1, Stride>::value ||
                                                                          is_integral_constant_of<-1, Offset>::value))>
                shift(T &obj, Stride const &stride, Offset offset) {
                obj += stride * offset;
            }

            // END `shift` PART

            /**
             *  Meta predicate that validates a single stride type against `shift` function
             */
            template <class T>
            struct is_valid_stride {
                template <class Stride>
                GT_META_DEFINE_ALIAS(
                    apply, std::is_void, decltype(void(shift(std::declval<T &>(), std::declval<Stride &>(), int_t{}))));
            };

            /**
             *  Meta predicate that validates a list of stride type against `shift` function
             */
            template <class StrideTypes, class T>
            GT_META_DEFINE_ALIAS(are_valid_strides, meta::all_of, (is_valid_stride<T>::template apply, StrideTypes));

            /**
             *  Sfinae unsafe version of `is_sid` predicate
             */
            template <class Sid,
                // Extracting all the derived types from Sid
                class Ptr = GT_META_CALL(ptr_type, Sid),
                class ReferenceType = GT_META_CALL(reference_type, Sid),
                class PtrDiff = GT_META_CALL(ptr_diff_type, Sid),
                class StridesType = GT_META_CALL(strides_type, Sid),
                class BoundsValidatorType = GT_META_CALL(bounds_validator_type, Sid),
                class StrideTypeList = GT_META_CALL(tuple_util::traits::to_types, StridesType)>
            GT_META_DEFINE_ALIAS(is_sid,
                conjunction,
                (
                    // `is_trivially_copyable` check is applied to the types that are will be passed from host to device
                    std::is_trivially_copyable<Ptr>,
                    std::is_trivially_copyable<StridesType>,
                    std::is_trivially_copyable<BoundsValidatorType>,

                    // verify that `PtrDiff` is sane
                    std::is_default_constructible<PtrDiff>,
                    std::is_same<decltype(std::declval<Ptr>() + PtrDiff{}), Ptr>,

                    // verify that `Reference` is sane
                    negation<std::is_void<ReferenceType>>,

                    // all strides must be applied via `shift` with both `Ptr` and `PtrDiff`
                    are_valid_strides<StrideTypeList, Ptr>,
                    are_valid_strides<StrideTypeList, PtrDiff>,

                    // `BoundsValidators` apllied to `Ptr` should return `bool`
                    std::is_constructible<bool,
                        decltype(std::declval<BoundsValidatorType const &>()(std::declval<Ptr const &>))>));

        } // namespace impl_

        // Meta functions
        using impl_::bounds_validator_kind;
        using impl_::bounds_validator_type;
        using impl_::ptr_diff_type;
        using impl_::ptr_type;
        using impl_::reference_type;
        using impl_::strides_kind;
        using impl_::strides_type;

        // Runtime functions
        using impl_::get_bounds_validator;
        using impl_::get_origin;
        using impl_::get_strides;
        using impl_::shift;

        /**
         *  Does the type models the SID concept
         */
        template <class T, class = void>
        struct is_sid : std::false_type {};
        template <class T>
        struct is_sid<T, enable_if_t<impl_::is_sid<T>::value>> : std::true_type {};

        // Auxiliary API

        /**
         *  The type of the element of the SID
         */
        template <class Sid, class Ref = GT_META_CALL(reference_type, Sid)>
        GT_META_DEFINE_ALIAS(element_type, meta::id, decay_t<Ref>);

        /**
         *  The const variation of the reference type
         */
        template <class Sid, class Ref = GT_META_CALL(reference_type, Sid)>
        GT_META_DEFINE_ALIAS(const_reference_type,
            meta::id,
            (conditional_t<std::is_reference<Ref>::value,
                add_lvalue_reference_t<add_const_t<remove_reference_t<Ref>>>,
                add_const_t<Ref>>));

        /**
         *  A Getter from Strides to the given stride.
         *
         *  If `I` exceeds the actual number of strides, std::integral_constant<int_t, 0> is returned.
         *  Which allows to silently ignore the offsets in non existing dimensions.
         */
        template <size_t I, class Strides, enable_if_t<(I < tuple_util::size<decay_t<Strides>>::value), int> = 0>
        constexpr GT_FUNCTION auto get_stride(Strides &&strides)
            GT_AUTO_RETURN(tuple_util::host_device::get<I>(strides));
        template <size_t I, class Strides, enable_if_t<(I >= tuple_util::size<decay_t<Strides>>::value), int> = 0>
        constexpr GT_FUNCTION std::integral_constant<int_t, 0> get_stride(Strides &&) {
            return {};
        }

    } // namespace sid

    /*
     *  Promote is_sid one level up.
     *
     *  Just because `sid::is_sid` looks a bit redundant
     */
    using sid::is_sid;
} // namespace gridtools