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

#include "../common/defs.hpp"
#include "../common/host_device.hpp"

#include "./arg.hpp"
#include "./block.hpp"
#include "./grid.hpp"
#include "./location_type.hpp"
#include "./pos3.hpp"

#include "./backend_cuda/tmp_storage.hpp"
#include "./backend_host/tmp_storage.hpp"

#ifdef STRUCTURED_GRIDS
#include "./structured_grids/tmp_storage.hpp"
#else
#include "./icosahedral_grids/tmp_storage.hpp"
#endif

namespace gridtools {
    namespace tmp_storage {
        template <class /*StorageInfo*/, class /*MaxExtent*/, class Backend>
        size_t get_k_size(Backend const &, size_t /*block_size*/, size_t total_size) {
            return total_size;
        }
        template <class /*StorageInfo*/, class /*MaxExtent*/, class Backend>
        GT_FUNCTION ptrdiff_t get_k_block_offset(Backend const &, size_t /*block_size*/, size_t /*block_no*/) {
            return 0;
        }
    } // namespace tmp_storage

    template <class MaxExtent, uint_t ArgId, class DataStore, int_t I, ushort_t NColors, class Backend, class Grid>
    DataStore make_tmp_data_store(
        Backend const &, arg<ArgId, DataStore, location_type<I, NColors>, true> const &, Grid const &grid) {
        GRIDTOOLS_STATIC_ASSERT(is_grid<Grid>::value, GT_INTERNAL_ERROR);
        using namespace tmp_storage;
        using storage_info_t = typename DataStore::storage_info_t;
        static constexpr auto backend = typename Backend::backend_ids_t{};
        return {make_storage_info<storage_info_t, NColors>(backend,
            get_i_size<storage_info_t, MaxExtent>(
                backend, block_i_size(backend, grid), grid.i_high_bound() - grid.i_low_bound() + 1),
            get_j_size<storage_info_t, MaxExtent>(
                backend, block_j_size(backend, grid), grid.j_high_bound() - grid.j_low_bound() + 1),
            get_k_size<storage_info_t, MaxExtent>(backend, block_k_size(backend, grid), grid.k_total_length()))};
    }

    template <class StorageInfo, class MaxExtent, class Backend, class Stride, class BlockNo, class PosInBlock>
    GT_FUNCTION ptrdiff_t get_tmp_storage_offset(Backend const &backend,
        Stride const &RESTRICT stride,
        BlockNo const &RESTRICT block_no,
        PosInBlock const &RESTRICT pos_in_block) {
        using namespace tmp_storage;
        static constexpr auto block_size =
            make_pos3(block_i_size(Backend{}), block_j_size(Backend{}), block_k_size(Backend{}));
        return stride.i *
                   (get_i_block_offset<StorageInfo, MaxExtent>(backend, block_size.i, block_no.i) + pos_in_block.i) +
               stride.j *
                   (get_j_block_offset<StorageInfo, MaxExtent>(backend, block_size.j, block_no.j) + pos_in_block.j) +
               stride.k *
                   (get_k_block_offset<StorageInfo, MaxExtent>(backend, block_size.k, block_no.k) + pos_in_block.k);
    };

} // namespace gridtools
