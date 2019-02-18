/*
 * GridTools Libraries
 * Copyright (c) 2019, ETH Zurich
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include "../../../common/defs.hpp"
#include "../../backend_ids.hpp"
#include "../../grid_traits_fwd.hpp"
#include "./execute_kernel_functor_cuda_fwd.hpp"

namespace gridtools {
    template <class Args>
    struct kernel_functor_executor<backend_ids<target::cuda, grid_type::icosahedral, strategy::block>, Args> {
        using type = icgrid::execute_kernel_functor_cuda<Args>;
    };
} // namespace gridtools
