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
#include <cassert>
#include <cstddef>
#include <type_traits>
#include <functional>
#include <vector>
#include <utility>
#include <memory>

#include <boost/mpl/logical.hpp>
#include <boost/mpl/transform.hpp>

#include <boost/fusion/include/count.hpp>
#include <boost/fusion/include/empty.hpp>
#include <boost/fusion/include/filter_if.hpp>
#include <boost/fusion/include/filter_view.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/front.hpp>
#include <boost/fusion/include/invoke.hpp>
#include <boost/fusion/include/make_fused.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/fusion/include/size.hpp>
#include <boost/fusion/include/transform.hpp>
#include <boost/fusion/include/zip_view.hpp>

#include "../../common/defs.hpp"
#include "../../common/vector_traits.hpp"
#include "../../common/functional.hpp"
#include "../../common/fusion.hpp"
#include "../../storage/data_store_field.hpp"
#include "../arg.hpp"
#include "../backend_metafunctions.hpp"
#include "../computation_grammar.hpp"
#include "../grid.hpp"
#include "../intermediate.hpp"
#include "../intermediate_impl.hpp"
#include "../mss_components_metafunctions.hpp"
#include "../conditionals/condition_tree.hpp"
#include "expand_factor.hpp"

namespace gridtools {

    namespace _impl {
        namespace expand_detail {
            template < typename T >
            struct is_expandable : std::false_type {};

            template < typename Arg, typename DataStoreType >
            struct is_expandable< arg_storage_pair< Arg, DataStoreType > > : is_vector< DataStoreType > {};

            template < uint_t N, typename T >
            struct convert_data_store_type {
                using type = T;
            };
            template < uint_t N, typename T >
            struct convert_data_store_type< N, std::vector< T > > {
                using type = data_store_field< T, N >;
            };

            template < uint_t N >
            struct convert_placeholder {
                template < typename >
                struct apply;
                template < uint_t I, typename S, typename L, bool T >
                struct apply< arg< I, S, L, T > > {
                    using type = arg< I, typename convert_data_store_type< N, S >::type, L, T >;
                };
            };

            template < uint_t N, typename Placeholders >
            using converted_placeholders =
                typename boost::mpl::transform< Placeholders, convert_placeholder< N > >::type;

            struct get_value_size {
                template < class T >
                size_t operator()(T const &t) const {
                    return t.m_value.size();
                }
#ifndef BOOST_RESULT_OF_USE_DECLTYPE
                using result_type = size_t;
#endif
            };

            template < typename ArgStoragePairs >
            size_t get_expandable_size(ArgStoragePairs const &src) {
                namespace f = boost::fusion;
                namespace m = boost::mpl;
                auto sizes = f::transform(
                    f::filter_if< m::and_< is_expandable< m::_ >, boost::mpl::not_< is_tmp_arg< m::_ > > > >(src),
                    get_value_size{});
                if (f::empty(sizes))
                    // If there is nothing to expand we are going to compute stensil once.
                    return 1;
                size_t res = f::front(sizes);
                assert(
                    f::count(sizes, res) == f::size(sizes) && "Non-tmp expandable parameters must have the same size");
                return res;
            }

            template < uint_t N >
            struct convert_data_store {
                template < typename T >
                data_store_field< T, N > operator()(const std::vector< T > &src) const {
                    assert(!src.empty());
                    data_store_field< T, N > res = {*src[0].get_storage_info_ptr()};
                    for (uint_t i = 0; i != N; ++i)
                        res.set(0, i, src[i]);
                    return res;
                }
                template < typename T >
                T operator()(const T &src) const {
                    return src;
                }
            };

            template < uint_t N >
            struct convert_arg_storage_pair {
                template < typename Arg, typename DataStoreType >
                arg_storage_pair< typename convert_placeholder< N >::template apply< Arg >::type,
                    typename convert_data_store_type< N, DataStoreType >::type >
                operator()(arg_storage_pair< Arg, DataStoreType > const &src) const {
                    return {convert_data_store< N >()(src.m_value)};
                }
                template < typename T >
                T operator()(T const &src) const {
                    return src;
                }
#ifndef BOOST_RESULT_OF_USE_DECLTYPE
                template < typename >
                struct result;
                template < typename Arg, typename DataStoreType >
                struct result< convert_arg_storage_pair(arg_storage_pair< Arg, DataStoreType > const &) > {
                    using type = arg_storage_pair< typename convert_placeholder< N >::template apply< Arg >::type,
                        typename convert_data_store_type< N, DataStoreType >::type >;
                };
                template < typename T >
                struct result< convert_arg_storage_pair(T const &) > {
                    using type = T;
                };
#endif
            };

            template < uint_t N,
                typename Aggregator,
                typename Res = aggregator_type< converted_placeholders< N, typename Aggregator::placeholders_t > > >
            Res convert_aggregator(const Aggregator &src) {
                namespace f = boost::fusion;
                namespace m = boost::mpl;
                return f::invoke(ctor< Res >{},
                    f::transform(f::filter_if< m::not_< is_tmp_arg< m::_ > > >(src.get_arg_storage_pairs()),
                                     convert_arg_storage_pair< N >()));
            }

            template < uint_t N >
            struct convert_mss_descriptors_tree_f {
                template < typename T >
                auto operator()(T const &src) const
                    GT_AUTO_RETURN((condition_tree_transform(src, fix_mss_arg_indices_f< N >{})));
            };

            template < uint_t N, typename MssDescriptorsTree >
            auto convert_mss_descriptors_tree(MssDescriptorsTree const &src)
                GT_AUTO_RETURN(convert_mss_descriptors_tree_f< N >{}(src));

            template < uint_t N, typename MssDescriptorsTree >
            using converted_mss_descriptors_tree =
                typename std::result_of< convert_mss_descriptors_tree_f< N >(MssDescriptorsTree const &) >::type;

            struct assign_storage {
                size_t m_offset;
                template < typename T, uint_t N >
                void operator()(std::vector< T > const &src, data_store_field< T, N > &dst) const {
                    assert(src.size() >= m_offset + N);
                    for (uint_t i = 0; i != N; ++i)
                        dst.set(0, i, src[m_offset + i]);
                }
                template < typename T >
                void operator()(T const &, T &) const {}
            };

            struct assign_arg_storage_pair {
                assign_storage m_assign_storage;
                template < typename Src, typename Dst >
                void operator()(Src const &src, Dst &dst) const {
                    m_assign_storage(src.m_value, dst.m_value);
                }
#ifndef BOOST_RESULT_OF_USE_DECLTYPE
                using result_type = void;
#endif
            };

            template < typename Src, typename Dst >
            void assign(const Src &src_all, Dst &dst_all, size_t offset) {
                namespace f = boost::fusion;
                namespace m = boost::mpl;
                using pred_t = m::not_< is_tmp_arg< m::_ > >;
                auto src = make_filter_view< pred_t >(src_all);
                auto dst = make_filter_view< pred_t >(dst_all);
                f::for_each(make_zip_view(f::make_vector(std::cref(src), std::ref(dst))),
                    f::make_fused(assign_arg_storage_pair{offset}));
            }
        }
    }
    /**
     * @file
     * \brief this file contains the intermediate representation used in case of expandable parameters
     * */

    /**
       @brief the intermediate representation object

       The expandable parameters are long lists of storages on which the same stencils are applied,
       in a Single-Stencil-Multiple-Storage way. In order to avoid resource contemption usually
       it is convenient to split the execution in multiple stencil, each stencil operating on a chunk
       of the list. Say that we have an expandable parameters list of length 23, and a chunk size of
       4, we'll execute 5 stencil with a "vector width" of 4, and one stencil with a "vector width"
       of 3 (23%4).

       This object contains two unique pointers of @ref gridtools::intermediate type, one with a vector width
       corresponding to the expand factor defined by the user (4 in the previous example), and another
       one with a vector width of expand_factor%total_parameters (3 in the previous example).
       In case the total number of parameters is a multiple of the expand factor, the second
       intermediate object does not get instantiated.
     */
    template < typename ExpandFactor, bool IsStateful, typename Backend, typename Grid, typename... MssDescriptorTrees >
    class intermediate_expand {
        GRIDTOOLS_STATIC_ASSERT((is_expand_factor< ExpandFactor >::value), GT_INTERNAL_ERROR);
        GRIDTOOLS_STATIC_ASSERT((is_backend< Backend >::value), GT_INTERNAL_ERROR);
        GRIDTOOLS_STATIC_ASSERT((is_grid< Grid >::value), GT_INTERNAL_ERROR);
        GRIDTOOLS_STATIC_ASSERT((boost::mpl::and_< std::true_type,
                                    is_condition_tree_of< MssDescriptorTrees, is_computation_token >... >::value),
            "make_computation args should be mss descriptors or condition trees of mss descriptors");

        using all_mss_descriptors_t = typename branch_selector< MssDescriptorTrees... >::all_leaves_t;
        using placeholders_t = typename extract_placeholders< all_mss_descriptors_t >::type;
        using Aggregator = aggregator_type< placeholders_t >;

        template < uint_t N >
        using converted_intermediate = intermediate< N,
            IsStateful,
            Backend,
            Grid,
            _impl::expand_detail::converted_mss_descriptors_tree< N, MssDescriptorTrees >... >;

        using arg_storage_pair_fusion_list_t = typename Aggregator::arg_storage_pair_fusion_list_t;

        // private members
        arg_storage_pair_fusion_list_t m_arg_storage_pairs;
        const size_t m_size;
        std::unique_ptr< converted_intermediate< ExpandFactor::value > > m_intermediate;
        // For some reason nvcc goes nuts here (even though the previous line is OK):
        // std::unique_ptr< converted_intermediate< 1 > > m_intermediate_remainder;
        // I have to expand `converted_intermediate` alias manually:
        std::unique_ptr< intermediate< 1,
            IsStateful,
            Backend,
            Grid,
            _impl::expand_detail::converted_mss_descriptors_tree< 1, MssDescriptorTrees >... > >
            m_intermediate_remainder;

      public:
        /**
           @brief constructor

           Given expandable parameters with size N, creates other @ref gristools::expandable_parameters storages with
           dimension given by  @ref gridtools::expand_factor
         */
        intermediate_expand(
            Grid const &grid, Aggregator const &domain, MssDescriptorTrees const &... mss_descriptor_trees)
            : m_arg_storage_pairs(domain.get_arg_storage_pairs()),
              m_size(_impl::expand_detail::get_expandable_size(m_arg_storage_pairs)),
              m_intermediate(m_size >= ExpandFactor::value
                                 ? create_intermediate< ExpandFactor::value >(grid, domain, mss_descriptor_trees...)
                                 : nullptr),
              m_intermediate_remainder(m_size % ExpandFactor::value
                                           ? create_intermediate< 1 >(grid, domain, mss_descriptor_trees...)
                                           : nullptr) {}

        /**
           @brief run the execution

           This method performs a run for the computation on each chunck of expandable parameters.
           Between two iterations it updates the @ref gridtools::aggregator_type, so that the storage
           pointers for the current chunck get substituted by the next chunk. At the end of the
           iterations, if the number of parameters is not multiple of the expand factor, the remaining
           chunck of storage pointers is consumed.
         */
        notype run() {
            size_t i = 0;
            for (; m_size - i >= ExpandFactor::value; i += ExpandFactor::value) {
                assign(*m_intermediate, i);
                m_intermediate->steady();
                m_intermediate->run();
            }
            for (; i < m_size; ++i) {
                assign(*m_intermediate_remainder, i);
                m_intermediate_remainder->steady();
                m_intermediate_remainder->run();
            }
            return {};
        }

        /**
           @brief forwards to the m_intermediate member

           does not take into account the remainder kernel executed when the number of parameters is
           not multiple of the expand factor
         */
        std::string print_meter() {
            assert(false);
            return {};
        }

        /**
           @brief forwards to the m_intermediate and m_intermediate_remainder members
         */
        void reset_meter() {
            if (m_intermediate)
                m_intermediate->reset_meter();
            if (m_intermediate_remainder)
                m_intermediate_remainder->reset_meter();
        }

        double get_meter() {
            double res = 0;
            if (m_intermediate)
                res += m_intermediate->get_meter();
            if (m_intermediate_remainder)
                res += m_intermediate_remainder->get_meter();
            return res;
        }

        void steady() {}

        void sync_all() {
            // sync all data stores
            boost::fusion::for_each(m_arg_storage_pairs, _impl::sync_data_stores());
        }

      private:
        template < uint_t N, typename Res = converted_intermediate< N > >
        static Res *create_intermediate(
            Grid const &grid, const Aggregator &domain, MssDescriptorTrees const &... mss_descriptor_trees) {
            return new Res(grid,
                _impl::expand_detail::convert_aggregator< N >(domain),
                _impl::expand_detail::convert_mss_descriptors_tree< N >(mss_descriptor_trees)...);
        }

        template < typename Dst >
        void assign(Dst &dst, size_t offset) const {
            _impl::expand_detail::assign(m_arg_storage_pairs, dst.get_arg_storage_pairs(), offset);
        }
    };
}
