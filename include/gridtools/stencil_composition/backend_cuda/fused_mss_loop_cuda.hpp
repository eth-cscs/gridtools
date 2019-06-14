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

#include <type_traits>

#include "../../common/defs.hpp"
#include "../../common/host_device.hpp"
#include "../../common/tuple_util.hpp"
#include "../../meta.hpp"
#include "../execution_types.hpp"
#include "../grid.hpp"
#include "../local_domain.hpp"
#include "../mss_components.hpp"
#include "basic_token_execution_cuda.hpp"
#include "iterate_domain_cuda.hpp"
#include "launch_kernel.hpp"

namespace gridtools {
    namespace cuda {
        namespace fused_mss_loop_cuda_impl_ {
            template <class ExecutionType>
            std::enable_if_t<!execute::is_parallel<ExecutionType>::value, int_t> blocks_required_z(uint_t) {
                return 1;
            }

            template <class ExecutionType>
            std::enable_if_t<execute::is_parallel<ExecutionType>::value, int_t> blocks_required_z(uint_t nz) {
                return (nz + ExecutionType::block_size - 1) / ExecutionType::block_size;
            }

            template <class ExecutionType, class From, class Grid>
            GT_FUNCTION_DEVICE std::enable_if_t<!execute::is_parallel<ExecutionType>::value, int_t> compute_kblock(
                Grid const &grid) {
                return grid.template value_at<From>();
            };

            template <class ExecutionType, class From, class Grid>
            GT_FUNCTION_DEVICE std::enable_if_t<execute::is_parallel<ExecutionType>::value, int_t> compute_kblock(
                Grid const &grid) {
                return max(blockIdx.z * ExecutionType::block_size, grid.template value_at<From>());
            };

            template <class RunFunctorArguments, int_t BlockSizeI, int_t BlockSizeJ, class LocalDomain, class Grid>
            struct kernel_f {
                using execution_type_t = typename RunFunctorArguments::execution_type_t;
                using iterate_domain_t = iterate_domain<LocalDomain, typename RunFunctorArguments::esf_sequence_t>;

                GT_STATIC_ASSERT(std::is_trivially_copyable<LocalDomain>::value, GT_INTERNAL_ERROR);
                GT_STATIC_ASSERT(std::is_trivially_copyable<Grid>::value, GT_INTERNAL_ERROR);

                LocalDomain m_local_domain;
                Grid m_grid;

                GT_FUNCTION_DEVICE void operator()(int_t iblock, int_t jblock) const {
                    using interval_t = meta::first<typename RunFunctorArguments::loop_intervals_t>;
                    using from_t = meta::first<interval_t>;

                    // number of threads
                    auto nx = m_grid.i_size();
                    auto ny = m_grid.j_size();
                    auto block_size_i = (blockIdx.x + 1) * BlockSizeI < nx ? BlockSizeI : nx - blockIdx.x * BlockSizeI;
                    auto block_size_j = (blockIdx.y + 1) * BlockSizeJ < ny ? BlockSizeJ : ny - blockIdx.y * BlockSizeJ;

                    iterate_domain_t it_domain(m_local_domain,
                        block_size_i,
                        block_size_j,
                        iblock,
                        jblock,
                        compute_kblock<execution_type_t, from_t>(m_grid));

                    // execute the k interval functors
                    run_functors_on_interval<RunFunctorArguments>(it_domain, m_grid);
                }
            };
        } // namespace fused_mss_loop_cuda_impl_

        template <class RunFunctorArguments, int_t BlockSizeI, int_t BlockSizeJ, class LocalDomain, class Grid>
        fused_mss_loop_cuda_impl_::kernel_f<RunFunctorArguments, BlockSizeI, BlockSizeJ, LocalDomain, Grid> make_kernel(
            LocalDomain const &local_domain, Grid const &grid) {
            return {local_domain, grid};
        }
    } // namespace cuda
} // namespace gridtools
