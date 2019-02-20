/*
 * GridTools
 *
 * Copyright (c) 2019, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "../../common/defs.hpp"
#include "../../common/pair.hpp"
#include "../backend_ids.hpp"
#include "../basic_token_execution.hpp"
#include "../execution_types.hpp"

namespace gridtools {
    template <class FromLevel, class ToLevel, class GridBackend, class Strategy, class ExecutionEngine, class Grid>
    GT_FUNCTION pair<int, int> get_k_interval(
        backend_ids<target::x86, GridBackend, Strategy>, ExecutionEngine, Grid const &grid) {
        return make_pair(grid.template value_at<FromLevel>(), grid.template value_at<ToLevel>());
    }
} // namespace gridtools
