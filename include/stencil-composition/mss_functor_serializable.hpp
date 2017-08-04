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

#include "mss_functor.hpp"
#include "../common/stencil_serializer.hpp"
#include "../common/type_name.hpp"
#include <boost/fusion/include/find.hpp>

namespace gridtools {

    namespace _impl {

        template < class SerializerType, class SavepointType, class StorageInfoList, class Domain >
        struct serialize_storages {
            SerializerType &m_serializer;
            SavepointType &m_savepoint;
            int_t *m_tmp_id;
            StorageInfoList &m_storage_info_ptr_list;
            const Domain &domain;

            template < typename Arg, typename Ptr >
            void operator()(boost::fusion::pair< Arg, Ptr > const &storage_ptr) const {
                using data_store_t = const typename Arg::storage_t;
                using storage_info_t = const typename data_store_t::storage_info_t;
                using storage_t = typename data_store_t::storage_t;

                if (Arg::is_temporary) {
                    auto storage_ptrs = storage_ptr.second;
                    auto storage_info_ptr = *boost::fusion::find< storage_info_t * >(m_storage_info_ptr_list);

                    // create data_store in external ptr mode to be used by the serializer
                    storage_t storage(0, storage_ptrs[0]);
                    data_store_t data_store(*storage_info_ptr, storage_ptrs[0]);
                    std::string tmp_name("tmp_" + std::to_string((*m_tmp_id)++));
                    m_serializer.write(tmp_name, m_savepoint, data_store);
                } else {
                    // take the storage from the domain (because the storage_ptr only contains raw ptr to data)
                    const auto &data_store_ =
                        *boost::fusion::at_c< Arg::index_t::value >(domain.get_arg_storage_pairs()).ptr;
                    m_serializer.write(data_store_.name(), m_savepoint, data_store_);
                }
            }
        };

    } // namespace _impl

    /**
     * \brief mss_functor with serialization capabilities
     */
    template < typename MssComponentsArray,
        typename Domain,
        typename Grid,
        typename MssLocalDomainArray,
        typename BackendIds,
        typename ReductionData,
        typename SerializerType >
    struct mss_functor_serializable
        : public mss_functor< MssComponentsArray, Grid, MssLocalDomainArray, BackendIds, ReductionData > {
      private:
        stencil_serializer< SerializerType > &m_stencil_serializer;
        const Domain &m_domain;

      public:
        using base_t = mss_functor< MssComponentsArray, Grid, MssLocalDomainArray, BackendIds, ReductionData >;

        mss_functor_serializable(MssLocalDomainArray &local_domain_lists,
            const Domain &domain,
            const Grid &grid,
            ReductionData &reduction_data,
            const int block_idx,
            const int block_idy,
            stencil_serializer< SerializerType > &stencil_ser)
            : base_t(local_domain_lists, grid, reduction_data, block_idx, block_idy), m_stencil_serializer(stencil_ser),
              m_domain(domain) {}

        template < typename Index >
        void operator()(Index const &index) const {
            SerializerType &serializer = m_stencil_serializer.get_serializer();

            // Query the local domain
            typedef typename boost::fusion::result_of::value_at< MssLocalDomainArray, Index >::type mss_local_domain_t;
            typedef typename mss_local_domain_list< mss_local_domain_t >::type local_domain_list_t;
            typedef typename boost::mpl::back< local_domain_list_t >::type local_domain_t;
            typedef typename boost::mpl::at< typename MssComponentsArray::elements, Index >::type mss_components_t;

            local_domain_list_t &local_domain_list =
                (local_domain_list_t &)boost::fusion::at< Index >(this->m_local_domain_lists).local_domain_list;
            local_domain_t &local_domain =
                (local_domain_t &)boost::fusion::at< boost::mpl::int_< 0 > >(local_domain_list);

            // Get current functor
            typedef typename boost::mpl::at_c< typename mss_components_t::functors_list_t, 0 >::type functor_pair_t;
            typedef typename functor_pair_t::f_type functor_t;

            auto stage_id = m_stencil_serializer.get_and_increment_stage_id();
            auto invocation_count = m_stencil_serializer.stencil_invocation_count();
            std::string stage_name(type_name< functor_t >());

            // This case has to be handled if extended to other backends
            static_assert(boost::mpl::size< typename mss_components_t::functors_list_t >::type::value == 1,
                "fused mss are currently not supported");

            // Create the input savepoint of the current stage
            typedef typename SerializerType::savepoint_t savepoint_t;
            savepoint_t savepoint_in(m_stencil_serializer.get_stencil_name() + "__in");
            savepoint_in.add_meta_info("stage_id", stage_id);
            savepoint_in.add_meta_info("stage_name", stage_name);
            savepoint_in.add_meta_info("invocation_count", invocation_count);

            // Serialize all input storages
            int_t tmp_id = 0;
            boost::fusion::for_each(
                local_domain.m_local_data_ptrs,
                _impl::serialize_storages< SerializerType,
                    savepoint_t,
                    typename local_domain_t::storage_info_ptr_fusion_list,
                    Domain >{serializer, savepoint_in, &tmp_id, local_domain.m_local_storage_info_ptrs, m_domain});

            // Run the functor
            //
            // TODO: if support for the CUDA backend is added, there should be proper synchronization here
            base_t::operator()(index);

            // Serialize output storages at the output savepoint
            savepoint_t savepoint_out(m_stencil_serializer.get_stencil_name() + "__out");
            savepoint_out.add_meta_info("stage_id", stage_id);
            savepoint_out.add_meta_info("stage_name", stage_name);
            savepoint_out.add_meta_info("invocation_count", invocation_count);

            tmp_id = 0;
            boost::fusion::for_each(
                local_domain.m_local_data_ptrs,
                _impl::serialize_storages< SerializerType,
                    savepoint_t,
                    typename local_domain_t::storage_info_ptr_fusion_list,
                    Domain >{serializer, savepoint_out, &tmp_id, local_domain.m_local_storage_info_ptrs, m_domain});
        }
    };

} // namespace gridtools
