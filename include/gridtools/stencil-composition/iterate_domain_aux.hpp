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

#include "../common/array.hpp"
#include "../common/generic_metafunctions/for_each.hpp"
#include "../common/generic_metafunctions/meta.hpp"
#include "../common/generic_metafunctions/static_if.hpp"
#include "arg_metafunctions.hpp"
#include "block.hpp"
#include "expressions/expressions.hpp"
#include "offset_computation.hpp"
#include "pos3.hpp"
#include "run_functor_arguments.hpp"
#include "tmp_storage.hpp"
#include <boost/fusion/include/pair.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/has_key.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/max_element.hpp>
#include <boost/mpl/modulus.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/size.hpp>
#include <boost/utility/enable_if.hpp>

/**
   @file
   @brief file implementing helper functions which are used in iterate_domain to assign/increment strides, access
   indices and storage pointers.

   All the helper functions use template recursion to implement loop unrolling
*/

namespace gridtools {

    namespace _impl {
        template <uint_t Coordinate, class StorageInfo>
        struct is_dummy_coordinate : bool_constant<(StorageInfo::layout_t::template at_unsafe<Coordinate>() < 0)> {};

        template <class StorageInfo, class LocalDomain>
        struct get_index : meta::st_position<typename LocalDomain::storage_info_list, StorageInfo> {};

        template <uint_t Coordinate,
            class StorageInfo,
            class Strides,
            int Cur = StorageInfo::layout_t::template at_unsafe<Coordinate>(),
            int Max = StorageInfo::layout_t::max(),
            enable_if_t<Cur<0, int> = 0> GT_FUNCTION int_t get_stride(Strides const &) {
            return 0;
        }

        template <uint_t Coordinate,
            class StorageInfo,
            class Strides,
            int Cur = StorageInfo::layout_t::template at_unsafe<Coordinate>(),
            int Max = StorageInfo::layout_t::max(),
            enable_if_t<Cur >= 0 && Cur == Max, int> = 0>
        GT_FUNCTION int_t get_stride(Strides const &) {
            return 1;
        }

        template <uint_t Coordinate,
            class StorageInfo,
            class Strides,
            int Cur = StorageInfo::layout_t::template at_unsafe<Coordinate>(),
            int Max = StorageInfo::layout_t::max(),
            enable_if_t<Cur >= 0 && Cur != Max, int> = 0>
        GT_FUNCTION int_t get_stride(Strides const &RESTRICT strides) {
            return boost::fusion::at_key<StorageInfo>(strides)[Cur];
        }
    } // namespace _impl

    /**@brief incrementing all the storage pointers to the m_data_pointers array

       @tparam Coordinate direction along which the increment takes place
       @tparam Strides strides cached type

           This method is responsible of incrementing the index for the memory access at
           the location (i,j,k) incremented/decremented by 1 along the 'Coordinate' direction. Such index is shared
           among all the fields contained in the
           same storage class instance, and it is not shared among different storage instances.
    */
    template <typename LocalDomain, uint_t Coordinate, typename Strides, typename ArrayIndex>
    struct increment_index_functor {
        GRIDTOOLS_STATIC_ASSERT((is_array_of<ArrayIndex, int>::value), GT_INTERNAL_ERROR);

        const int_t m_increment;
        ArrayIndex &RESTRICT m_index_array;
        Strides const &RESTRICT m_strides;

        using storage_info_list = typename LocalDomain::storage_info_list;

        template <typename Index, typename StorageInfo = GT_META_CALL(meta::at, (storage_info_list, Index))>
        GT_FUNCTION enable_if_t<_impl::is_dummy_coordinate<Coordinate, StorageInfo>::value> operator()(Index) const {}

        template <typename Index, typename StorageInfo = GT_META_CALL(meta::at, (storage_info_list, Index))>
        GT_FUNCTION enable_if_t<!_impl::is_dummy_coordinate<Coordinate, StorageInfo>::value> operator()(Index) const {
            m_index_array[Index::value] += _impl::get_stride<Coordinate, StorageInfo>(m_strides) * m_increment;
        }
    };

    template <uint_t Coordinate, class LocalDomain, class Strides, class ArrayIndex>
    GT_FUNCTION void do_increment(
        int_t step, LocalDomain const &local_domain, Strides const &RESTRICT strides, ArrayIndex &index) {
        gridtools::for_each<GT_META_CALL(meta::make_indices_for, typename LocalDomain::storage_info_list)>(
            increment_index_functor<LocalDomain, Coordinate, Strides, ArrayIndex>{step, index, strides});
    }

    template <uint_t Coordinate, ptrdiff_t Step, class LocalDomain, class Strides, class ArrayIndex>
    GT_FUNCTION void do_increment(LocalDomain const &local_domain, Strides const &RESTRICT strides, ArrayIndex &index) {
        gridtools::for_each<GT_META_CALL(meta::make_indices_for, typename LocalDomain::storage_info_list)>(
            increment_index_functor<LocalDomain, Coordinate, Strides, ArrayIndex>{Step, index, strides});
    }

    /**@brief functor initializing the indices does the actual assignment
     *     This method is responsible of computing the index for the memory access at
     *     the location (i,j,k). Such index is shared among all the fields contained in the
     *     same storage class instance, and it is not shared among different storage instances.
     * @tparam Coordinate direction along which the increment takes place
     * @tparam Strides tuple of strides arrays
     * @tparam StorageSequence sequence of storages
     */
    template <class StorageInfo, class MaxExtent, bool IsTmp>
    struct get_index_offset_f;

    template <class StorageInfo, class MaxExtent>
    struct get_index_offset_f<StorageInfo, MaxExtent, false> {
        template <class Backend, class Stride, class Begin, class BlockNo, class PosInBlock>
        GT_FUNCTION int_t operator()(Backend const &,
            Stride const &RESTRICT stride,
            Begin const &RESTRICT begin,
            BlockNo const &RESTRICT block_no,
            PosInBlock const &RESTRICT pos_in_block) const {
            static constexpr auto block_size =
                make_pos3(block_i_size(Backend{}), block_j_size(Backend{}), block_k_size(Backend{}));
            return stride.i * (begin.i + block_no.i * block_size.i + pos_in_block.i) +
                   stride.j * (begin.j + block_no.j * block_size.j + pos_in_block.j) +
                   stride.k * (begin.k + block_no.k * block_size.k + pos_in_block.k);
        }
    };

    template <class StorageInfo, class MaxExtent>
    struct get_index_offset_f<StorageInfo, MaxExtent, true> {
        template <class Backend, class Stride, class Begin, class BlockNo, class PosInBlock>
        GT_FUNCTION int_t operator()(Backend const &backend,
            Stride const &RESTRICT stride,
            Begin const &RESTRICT /*begin*/,
            BlockNo const &RESTRICT block_no,
            PosInBlock const &RESTRICT pos_in_block) const {
            return get_tmp_storage_offset<StorageInfo, MaxExtent>(backend, stride, block_no, pos_in_block);
        }
    };

    template <class Strides, class LocalDomain, class ArrayIndex, class Backend>
    struct initialize_index_f {
        GRIDTOOLS_STATIC_ASSERT((is_array_of<ArrayIndex, int>::value), GT_INTERNAL_ERROR);
        Strides const &RESTRICT m_strides;
        pos3<uint_t> m_begin;
        pos3<uint_t> m_block_no;
        pos3<int_t> m_pos_in_block;
        ArrayIndex &RESTRICT m_index_array;

        template <typename Index>
        GT_FUNCTION void operator()(Index) const {
            using max_extent_t = typename LocalDomain::max_extent_for_tmp_t;
            using storage_info_t = GT_META_CALL(meta::at, (typename LocalDomain::storage_info_list, Index));
            using layout_t = typename storage_info_t::layout_t;
            static constexpr auto backend = Backend{};
            static constexpr auto is_tmp =
                meta::st_contains<typename LocalDomain::tmp_storage_info_list, storage_info_t>::value;
            m_index_array[Index::value] = get_index_offset_f<storage_info_t, max_extent_t, is_tmp>{}(backend,
                make_pos3(_impl::get_stride<coord_i<Backend>::value, storage_info_t>(m_strides),
                    _impl::get_stride<coord_j<Backend>::value, storage_info_t>(m_strides),
                    _impl::get_stride<coord_k<Backend>::value, storage_info_t>(m_strides)),
                m_begin,
                m_block_no,
                m_pos_in_block);
        }
    };

    /**
     * function that checks a given pointer and offset combination results in an out of bounds access.
     * the check is computing the fields offset in order to get the base address of the accessed storage.
     * once the base address is known it can be checked if the requested access lies within the
     * storages allocated memory.
     */
    template <typename StorageInfo, typename LocalDomain>
    GT_FUNCTION bool pointer_oob_check(LocalDomain const &local_domain, int_t offset) {
        return offset >= 0 && offset < boost::fusion::at_key<StorageInfo>(local_domain.m_local_padded_total_lengths);
    }

    /**
     * metafunction that evaluates if an accessor is cached by the backend
     * the Accessor parameter is either an Accessor or an expressions
     */
    template <typename Accessor, typename CachesMap>
    struct accessor_is_cached {
        template <typename Accessor_>
        struct accessor_is_cached_ {
            GRIDTOOLS_STATIC_ASSERT((is_accessor<Accessor>::value), GT_INTERNAL_ERROR);
            typedef typename boost::mpl::has_key<CachesMap, typename accessor_index<Accessor_>::type>::type type;
        };

        typedef typename boost::mpl::eval_if<is_accessor<Accessor>,
            accessor_is_cached_<Accessor>,
            boost::mpl::identity<boost::mpl::false_>>::type type;

        BOOST_STATIC_CONSTANT(bool, value = (type::value));
    };

    template <typename LocalDomain, typename Accessor>
    struct get_storage_accessor {
        GRIDTOOLS_STATIC_ASSERT(is_local_domain<LocalDomain>::value, GT_INTERNAL_ERROR);
        GRIDTOOLS_STATIC_ASSERT(is_accessor<Accessor>::value, GT_INTERNAL_ERROR);

        GRIDTOOLS_STATIC_ASSERT(
            (boost::mpl::size<typename LocalDomain::data_ptr_fusion_map>::value > Accessor::index_t::value),
            GT_INTERNAL_ERROR);
        typedef typename LocalDomain::template get_arg<typename Accessor::index_t>::type::data_store_t type;
    };

    /**
     * metafunction that retrieves the arg type associated with an accessor
     */
    template <typename Accessor, typename LocalDomain>
    struct get_arg_from_accessor {
        using type = typename LocalDomain::template get_arg<typename Accessor::index_t>::type;
    };

    template <typename Accessor, typename LocalDomain>
    struct get_arg_value_type_from_accessor {
        typedef typename get_arg_from_accessor<Accessor, LocalDomain>::type::data_store_t::data_t type;
    };

    /**
     * metafunction that computes the return type of all operator() of an accessor
     */
    template <typename Accessor, typename IterateDomainArguments>
    struct accessor_return_type_impl {
        typedef typename boost::remove_reference<Accessor>::type acc_t;

        typedef typename boost::mpl::eval_if<boost::mpl::or_<is_accessor<acc_t>, is_vector_accessor<acc_t>>,
            get_arg_value_type_from_accessor<acc_t, typename IterateDomainArguments::local_domain_t>,
            boost::mpl::identity<boost::mpl::void_>>::type accessor_value_type;

        typedef typename boost::mpl::if_<is_accessor_readonly<acc_t>,
            typename boost::add_const<accessor_value_type>::type,
            typename boost::add_reference<accessor_value_type>::type RESTRICT>::type type;
    };

    namespace aux {
        /**
         * @brief metafunction that determines if a given accessor is associated with a placeholder holding a datafield
         */
        template <typename Accessor, typename LocalDomain>
        struct accessor_holds_data_field {
            typedef typename boost::mpl::eval_if<is_accessor<Accessor>,
                arg_holds_data_field_h<get_arg_from_accessor<Accessor, LocalDomain>>,
                boost::mpl::identity<boost::mpl::false_>>::type type;
        };

        /**
         * @brief method returning the data pointer of an accessor
         *
         * Specialization for the accessor placeholders for standard storages.
         *
         * This method is enabled only if the current placeholder dimension does not exceed the number of space
         * dimensions of the storage class.
         * I.e., if we are dealing with storages, not with storage lists or data fields (see concepts page for
         * definitions).
         */
        template <typename LocalDomain,
            typename Accessor,
            typename ArgT = typename get_arg_from_accessor<Accessor, LocalDomain>::type,
            typename ReturnT = typename ArgT::type::data_store_t::data_t>
        GT_FUNCTION typename boost::disable_if<typename accessor_holds_data_field<Accessor, LocalDomain>::type,
            ReturnT * RESTRICT>::type
        get_data_pointer(LocalDomain const &local_domain, Accessor const &accessor) {
            using storage_info_t = typename ArgT::data_store_t::storage_info_t;

            GRIDTOOLS_STATIC_ASSERT(Accessor::n_dimensions <= storage_info_t::layout_t::masked_length,
                "requested accessor index lower than zero. Check that when you define the accessor you specify the "
                "dimenisons which you actually access. e.g. suppose that a storage linked to the accessor ```in``` has "
                "5 dimensions, and thus can be called with in(Dimensions<5>(-1)). Calling in(Dimensions<6>(-1)) brings "
                "you here.");

            typedef typename boost::remove_const<typename boost::remove_reference<Accessor>::type>::type acc_t;
            GRIDTOOLS_STATIC_ASSERT((is_accessor<acc_t>::value), "Using EVAL is only allowed for an accessor type");

            return boost::fusion::at_key<ArgT>(local_domain.m_local_data_ptrs)[0];
        }

        /**
         * @brief method returning the data pointer of an accessor
         *
         * Specialization for the accessor placeholder for extended storages,
         * containing multiple snapshots of data fields with the same dimension and memory layout)
         *
         * This method is enabled only if the current placeholder dimension exceeds the number of space dimensions of
         * the storage class. I.e., if we are dealing with  storage lists or data fields (see concepts page for
         * definitions).
         */
        template <typename LocalDomain,
            typename Accessor,
            typename ArgT = typename get_arg_from_accessor<Accessor, LocalDomain>::type,
            typename DataStoreT = typename ArgT::data_store_t,
            typename ReturnT = typename DataStoreT::data_t>
        GT_FUNCTION typename boost::enable_if<typename accessor_holds_data_field<Accessor, LocalDomain>::type,
            ReturnT * RESTRICT>::type
        get_data_pointer(LocalDomain const &local_domain, Accessor const &accessor) {
            GRIDTOOLS_STATIC_ASSERT((is_accessor<Accessor>::value), "Using EVAL is only allowed for an accessor type");
            using storage_info_t = typename DataStoreT::storage_info_t;

            GRIDTOOLS_STATIC_ASSERT(Accessor::n_dimensions == storage_info_t::layout_t::masked_length + 2,
                "The dimension of the data_store_field accessor must be equals to storage dimension + 2 (component and "
                "snapshot)");

            const int_t idx = get_datafield_offset<DataStoreT>::get(accessor);
            assert(
                idx < DataStoreT::num_of_storages && "Out of bounds access when accessing data store field element.");

            return boost::fusion::at_key<ArgT>(local_domain.m_local_data_ptrs)[idx];
        }
    } // namespace aux
} // namespace gridtools
