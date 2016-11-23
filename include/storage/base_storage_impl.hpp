/*
  GridTools Libraries

  Copyright (c) 2016, GridTools Consortium
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
#include <gridtools.hpp>
#include "common/defs.hpp"
#include <boost/lexical_cast.hpp>
#include "../common/is_temporary_storage.hpp"
#include "../common/generic_metafunctions/gt_integer_sequence.hpp"
#include "../common/generic_metafunctions/all_integrals.hpp"
#include "../common/offset_metafunctions.hpp"

namespace gridtools {

    template < typename Tuple >
    struct is_arg_tuple;

    namespace _impl {

#ifdef CXX11_ENABLED
        /**@brief metafunction to recursively compute the next stride
           ID goes from space_dimensions-2 to 0
           MaxIndex is space_dimensions-1
        */
        template < short_t ID, short_t MaxIndex, typename Layout >
        struct next_stride {

            template < typename First, typename... IntTypes >
            GT_FUNCTION static constexpr First apply(First first, IntTypes... args) {
                return Layout::template find_val< MaxIndex - ID, short_t, 1 >(first, args...) *
                       next_stride< ID - 1, MaxIndex, Layout >::apply(first, args...);
            }
        };

        /**@brief template specialization to stop the recursion*/
        template < short_t MaxIndex, typename Layout >
        struct next_stride< 0, MaxIndex, Layout > {
            template < typename First, typename... IntTypes >
            GT_FUNCTION static constexpr First apply(First first, IntTypes... args) {
                return Layout::template find_val< MaxIndex, short_t, 1 >(first, args...);
            }
        };

        /**@brief functor to assign all the strides */
        template < int_t MaxIndex, typename Layout >
        struct assign_all_strides {

            template < int_t T >
            using lambda = next_stride< MaxIndex - T, MaxIndex, Layout >;

            template < typename... UIntType, typename Dummy = all_integers< UIntType... > >
            static constexpr array< int_t, MaxIndex + 1 > apply(UIntType... args) {
                using seq =
                    apply_gt_integer_sequence< typename make_gt_integer_sequence< int_t, sizeof...(args) >::type >;
                return seq::template apply< array< int_t, MaxIndex + 1 >, lambda >((int_t)args...);
            }
        };

#endif

        /**@brief struct to compute the total offset (the sum of the i,j,k indices times their respective strides)
         */
        template < ushort_t Id, typename Layout >
        struct compute_offset {
            static const ushort_t space_dimensions = Layout::length;
            GRIDTOOLS_STATIC_ASSERT((is_layout_map< Layout >::value), "wrong type");
            /**interface with an array of coordinates as argument
               \param strides the strides
               \param indices the array of coordinates
            */
            template < typename IntType, typename StridesVector >
            GT_FUNCTION static constexpr int_t apply(StridesVector const &RESTRICT strides_, IntType *indices_) {
                return strides_[space_dimensions - Id] *
                           Layout::template find_val< space_dimensions - Id, int, 0 >(indices_) +
                       compute_offset< Id - 1, Layout >::apply(strides_, indices_);
            }

#ifdef CXX11_ENABLED
            /**interface with the coordinates as variadic arguments
               \param strides the strides
               \param indices comma-separated list of coordinates
            */
            template < typename StridesVector, typename... Int, typename Dummy = all_integers< Int... > >
            GT_FUNCTION static constexpr int_t apply(StridesVector const &RESTRICT strides_, Int const &... indices_) {
                return strides_[space_dimensions - Id] *
                           (Layout::template find_val< space_dimensions - Id, int_t, 0 >(indices_...)) +
                       compute_offset< Id - 1, Layout >::apply(strides_, indices_...);
            }

            /**interface with the coordinates as static integer variadic arguments
               \param strides the strides
               \param indices comma-separated list of coordinates
            */
            template < typename StridesVector, typename... UInt, typename Dummy = all_static_integers< UInt... > >
            GT_FUNCTION static constexpr int_t apply(StridesVector const &strides_, UInt... indices_) {
                return strides_[space_dimensions - Id] *
                           Layout::template find_val< space_dimensions - Id, int_t, 0 >(UInt::value...) +
                       compute_offset< Id - 1, Layout >::apply(strides_, UInt{}...);
            }
#endif

            /**interface with the coordinates as a tuple
               \param strides the strides
               \param indices tuple of coordinates
            */
            template < typename Offset, typename StridesVector >
            GT_FUNCTION static constexpr int_t apply(StridesVector const &RESTRICT strides_,
                Offset const &indices_,
                typename boost::enable_if< typename is_tuple_or_array< Offset >::type, int >::type * = 0) {
                return (int_t)strides_[space_dimensions - Id] *
                           Layout::template find_val< space_dimensions - Id, uint_t, 0 >(indices_) +
                       compute_offset< Id - 1, Layout >::apply(strides_, indices_);
            }
        };

        /**@brief stops the recursion
         */
        template < typename Layout >
        struct compute_offset< 1, Layout > {
            static const ushort_t space_dimensions = Layout::length;

            template < typename IntType, typename StridesVector >
            GT_FUNCTION static constexpr int_t apply(StridesVector const &RESTRICT /*strides*/, IntType *indices_) {
                return Layout::template find_val< space_dimensions - 1, int, 0 >(indices_);
            }

#ifdef CXX11_ENABLED
            template < typename StridesVector, typename... IntType, typename Dummy = all_integers< IntType... > >
            GT_FUNCTION static constexpr int_t apply(
                StridesVector const &RESTRICT /*strides*/, IntType const &... indices_) {
                return Layout::template find_val< space_dimensions - 1, int, 0 >(indices_...);
            }

            /**interface with the coordinates as variadic arguments
               \param strides the strides
               \param indices comma-separated list of coordinates
            */
            template < typename StridesVector, typename... UInt, typename Dummy = all_static_integers< UInt... > >
            GT_FUNCTION static constexpr int_t apply(StridesVector const &RESTRICT strides_, UInt... indices_) {
                return Layout::template find_val< space_dimensions - 1, int, 0 >(UInt::value...);
            }

#endif
            /**interface with the coordinates as a tuple
               \param strides the strides
               \param indices tuple of coordinates
            */
            template < typename Offset, typename StridesVector >
            GT_FUNCTION static constexpr int_t apply(StridesVector const &RESTRICT /*strides*/,
                Offset const &indices_,
                typename boost::enable_if< typename is_tuple_or_array< Offset >::type, int >::type * = 0) {
                return Layout::template find_val< space_dimensions - 1, int, 0 >(indices_);
            }
        };

        /**@brief metafunction to access a type sequence at a given position, numeration from 0

           he types in the sequence must define a 'super' type. Can be seen as a compile-time equivalent of a
           linked-list.
        */
        template < int_t ID, typename Sequence >
        struct access {
            BOOST_STATIC_ASSERT(ID > 0);
            // BOOST_STATIC_ASSERT(ID<=Sequence::n_fields);
            typedef typename access< ID - 1, typename Sequence::super >::type type;
        };

        /**@brief template specialization to stop the recursion*/
        template < typename Sequence >
        struct access< 0, Sequence > {
            typedef Sequence type;
        };

        /**@brief recursively advance the ODE finite difference for all the field dimensions*/
        template < short_t Dim >
        struct advance_recursive {
            template < typename This >
            GT_FUNCTION void apply(This *t) {
                t->template advance< Dim >();
                advance_recursive< Dim - 1 >::apply(t);
            }
        };

        /**@brief template specialization to stop the recursion*/
        template <>
        struct advance_recursive< 0 > {
            template < typename This >
            GT_FUNCTION void apply(This *t) {
                t->template advance< 0 >();
            }
        };

    } // namespace _impl

    template < enumtype::execution Execution >
    struct increment_policy;

    template <>
    struct increment_policy< enumtype::forward > {

        template < typename Lhs, typename Rhs >
        GT_FUNCTION static void apply(Lhs &lhs, Rhs const &rhs) {
            lhs += rhs;
        }
    };

    template <>
    struct increment_policy< enumtype::backward > {

        template < typename Lhs, typename Rhs >
        GT_FUNCTION static void apply(Lhs &lhs, Rhs const &rhs) {
            lhs -= rhs;
        }
    };

    namespace _debug {
#ifndef NDEBUG
        struct print_pointer {
            template < typename StorageType >
            GT_FUNCTION_WARNING void operator()(pointer< StorageType > s) const {
                printf("Pointer Value %x\n", s.get());
            }
        };
#endif
    } // namespace _debug
} // namesapace gridtools
