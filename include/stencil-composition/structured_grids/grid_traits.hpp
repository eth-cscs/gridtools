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

#include "../../common/numerics.hpp"
#include "../compute_extents_metafunctions.hpp"
#include "../storage_wrapper.hpp"
#include "../tile.hpp"
#include "grid_traits_backend_fwd.hpp"

#include <boost/mpl/fold.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/max_element.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/quote.hpp>
#include <boost/mpl/vector.hpp>

#ifdef __CUDACC__
#include "backend_cuda/grid_traits_cuda.hpp"
#else
#include "backend_host/grid_traits_host.hpp"
#endif

namespace gridtools {

    template <>
    struct grid_traits_from_id< enumtype::structured > {
        // index positions of the different dimensions in the layout map (convention)
        typedef static_uint< 0 > dim_i_t;
        typedef static_uint< 1 > dim_j_t;
        typedef static_uint< 2 > dim_k_t;

        struct select_mss_compute_extent_sizes {
            template < typename PlaceholdersMap, typename Mss, uint_t RepeatFunctor >
            struct apply {
                typedef
                    typename compute_extents_of< PlaceholdersMap, RepeatFunctor >::template for_mss< Mss >::type type;
            };
        };

        template < typename Placeholders >
        struct select_init_map_of_extents {
            typedef typename init_map_of_extents< Placeholders >::type type;
        };

        typedef extent< 0, 0, 0, 0 > null_extent_t;

        template < enumtype::platform BackendId >
        struct with_arch {
            typedef strgrid::grid_traits_arch< BackendId > type;
        };

        // get a temporary storage for Host Naive
        template < typename T, typename Backend, typename StorageWrapper, typename Grid >
        static typename boost::enable_if_c< (Backend::s_strategy_id == enumtype::Naive), T >::type
        instantiate_storage_info(Grid const &grid) {
            // get all the params (size in i,j,k and number of threads in i,j)
            const uint_t i_size = grid.direction_i().total_length();
            const uint_t j_size = grid.direction_j().total_length();
            const uint_t k_size = (grid.k_max() + 1);
            return T(i_size, j_size, k_size);
        }

        // get a temporary storage for Host Block
        template < typename T, typename Backend, typename StorageWrapper, typename Grid >
        static typename boost::enable_if_c< (Backend::s_strategy_id == enumtype::Block &&
                                                Backend::s_backend_id == enumtype::Host),
            T >::type
        instantiate_storage_info(Grid const &grid) {
            typedef boost::mpl::int_< T::halo_t::template at< dim_i_t::value >() > halo_i;

            // get all the params (size in i,j,k and number of threads in i,j)
            const uint_t k_size = (grid.k_max() + 1);
            const uint_t threads_i = Backend::n_i_pes()(grid.i_high_bound() - grid.i_low_bound());
            const uint_t threads_j = Backend::n_j_pes()(grid.j_high_bound() - grid.j_low_bound());

            // create and return the storage info instance
            return T((StorageWrapper::tileI_t::s_tile + 2 * halo_i::value) * threads_i - 2 * halo_i::value,
                (StorageWrapper::tileJ_t::s_tile)*threads_j,
                k_size);
        }

        // get a temporary storage for Cuda
        template < typename T, typename Backend, typename StorageWrapper, typename Grid >
        static typename boost::enable_if_c< (Backend::s_strategy_id == enumtype::Block &&
                                                Backend::s_backend_id == enumtype::Cuda),
            T >::type
        instantiate_storage_info(Grid const &grid) {
            typedef boost::mpl::int_< T::halo_t::template at< dim_i_t::value >() > halo_i;
            typedef boost::mpl::int_< T::halo_t::template at< dim_j_t::value >() > halo_j;

            // get all the params (size in i,j,k and number of threads in i,j)
            const uint_t k_size = (grid.k_max() + 1);
            const uint_t threads_i = Backend::n_i_pes()(grid.i_high_bound() - grid.i_low_bound());
            const uint_t threads_j = Backend::n_j_pes()(grid.j_high_bound() - grid.j_low_bound());

            constexpr int full_block_size = StorageWrapper::tileI_t::s_tile + 2 * halo_i::value;
            constexpr int diff_between_blocks =
                (T::alignment_t::value)
                    ? _impl::static_ceil(static_cast< float >(full_block_size) / T::alignment_t::value) *
                          T::alignment_t::value
                    : full_block_size;
            constexpr int padding_between_blocks = diff_between_blocks - full_block_size;
            const int inner_domain_size = threads_i * StorageWrapper::tileI_t::s_tile +
                                          (threads_i - 1) * (padding_between_blocks + 2 * halo_i::value);

            // create and return the storage info instance
            return T(inner_domain_size,
                (StorageWrapper::tileJ_t::s_tile + 2 * halo_j::value) * threads_j - 2 * halo_j::value,
                k_size);
        }
    };
}
