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
// //\todo this struct becomes redundant when the auto keyword is used
namespace gridtools {

    struct stencil {
        virtual void ready() = 0;
        virtual void steady() = 0;
        virtual void finalize() = 0;
        virtual notype run() = 0;
        virtual std::string print_meter() = 0;
        virtual double get_meter() = 0;
        virtual void reset_meter() = 0;
    };

    template < typename ReductionType >
    struct reduction {
        virtual void ready() = 0;
        virtual void steady() = 0;
        virtual void finalize() = 0;
        virtual ReductionType run() = 0;
        virtual std::string print_meter() = 0;
        virtual double get_meter() = 0;
        virtual void reset_meter() = 0;
    };

    template < typename Aggregator, typename ReductionType >
    struct computation
        : public boost::mpl::if_< boost::is_same< ReductionType, notype >, stencil, reduction< ReductionType > >::type {

      public:
        computation(Aggregator const &domain) : m_domain(domain) {}

        template < typename... DataStores,
            typename boost::enable_if< typename _impl::aggregator_storage_check< DataStores... >::type, int >::type =
                0 >
        void reassign(DataStores &... stores) {
            boost::fusion::for_each(m_domain.get_arg_storage_pairs(), _impl::sync_data_stores());
            m_domain.reassign_storages_impl(stores...);
        }

        template < typename... ArgStoragePairs,
            typename boost::enable_if< typename _impl::aggregator_arg_storage_pair_check< ArgStoragePairs... >::type,
                int >::type = 0 >
        void reassign(ArgStoragePairs... pairs) {
            boost::fusion::for_each(m_domain.get_arg_storage_pairs(), _impl::sync_data_stores());
            m_domain.reassign_arg_storage_pairs_impl(pairs...);
        }

      protected:
        Aggregator m_domain;
    };

} // namespace gridtools
