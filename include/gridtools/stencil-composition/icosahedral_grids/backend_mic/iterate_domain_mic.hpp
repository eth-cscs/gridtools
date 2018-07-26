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

#include <type_traits>

#include "../../iterate_domain_fwd.hpp"
#include "../iterate_domain.hpp"

namespace gridtools {
    /**
     * @brief iterate domain class for the Mic backend
     */
    template <typename IterateDomainArguments>
    class iterate_domain_mic
        : public iterate_domain<iterate_domain_mic<IterateDomainArguments>, IterateDomainArguments> // CRTP
    {
        DISALLOW_COPY_AND_ASSIGN(iterate_domain_mic);
        GRIDTOOLS_STATIC_ASSERT((is_iterate_domain_arguments<IterateDomainArguments>::value), GT_INTERNAL_ERROR);

        typedef iterate_domain<iterate_domain_mic<IterateDomainArguments>, IterateDomainArguments> super;

      public:
        typedef iterate_domain_mic iterate_domain_t;
        typedef typename super::local_domain_t local_domain_t;
        typedef typename super::grid_topology_t grid_topology_t;
        typedef boost::mpl::map0<> ij_caches_map_t;

        GT_FUNCTION
        explicit iterate_domain_mic(local_domain_t const &local_domain_, grid_topology_t const &grid_topology)
            : super(local_domain_, grid_topology) {}

        template <typename ReturnType, typename Accessor, typename StorageType>
        GT_FUNCTION ReturnType get_value_impl(
            StorageType *RESTRICT storage_pointer, const uint_t pointer_offset) const {
            GRIDTOOLS_STATIC_ASSERT((is_accessor<Accessor>::value), GT_INTERNAL_ERROR);

            return *(storage_pointer + pointer_offset);
        }

        /**
         * caches are not currently used in mic backend
         */
        template <typename IterationPolicy>
        GT_FUNCTION void slide_caches() {
            GRIDTOOLS_STATIC_ASSERT((is_iteration_policy<IterationPolicy>::value), "error");
        }

        /**
         * caches are not currently used in mic backend
         */
        template <typename IterationPolicy, typename Grid>
        GT_FUNCTION void flush_caches(const int_t klevel, Grid const &grid) {
            GRIDTOOLS_STATIC_ASSERT((is_iteration_policy<IterationPolicy>::value), "error");
            GRIDTOOLS_STATIC_ASSERT((is_grid<Grid>::value), "error");
        }

        /**
         * caches are not currently used in mic backend
         */
        template <typename IterationPolicy, typename Grid>
        GT_FUNCTION void fill_caches(const int_t klevel, Grid const &grid) {
            GRIDTOOLS_STATIC_ASSERT((is_iteration_policy<IterationPolicy>::value), "error");
            GRIDTOOLS_STATIC_ASSERT((is_grid<Grid>::value), "error");
        }

        /**
         * caches are not currently used in mic backend
         */
        template <typename IterationPolicy>
        GT_FUNCTION void final_flush() {
            GRIDTOOLS_STATIC_ASSERT((is_iteration_policy<IterationPolicy>::value), "error");
        }

        /**
         * caches are not currently used in mic backend
         */
        template <typename IterationPolicy>
        GT_FUNCTION void begin_fill() {
            GRIDTOOLS_STATIC_ASSERT((is_iteration_policy<IterationPolicy>::value), "error");
        }

        template <typename Extent>
        GT_FUNCTION bool is_thread_in_domain() const {
            return true;
        }
    };

    template <typename IterateDomainArguments>
    struct is_iterate_domain<iterate_domain_mic<IterateDomainArguments>> : std::true_type {};

} // namespace gridtools
