/*
 * GridTools
 *
 * Copyright (c) 2014-2019, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "interpolate_stencil.hpp"

#include <gridtools/stencil_composition/expressions/expressions.hpp>

struct interpolate_stage {
    using in1 = gridtools::accessor<0, gridtools::intent::in>;
    using in2 = gridtools::accessor<1, gridtools::intent::in>;
    using weight = gridtools::global_accessor<2>;
    using out = gridtools::accessor<3, gridtools::intent::inout>;

    using param_list = gridtools::make_param_list<in1, in2, weight, out>;

    template <typename Evaluation>
    GT_FUNCTION static void apply(Evaluation eval) {
        using namespace gridtools::expressions;

        eval(out()) = eval(weight() * in1() + (1. - weight()) * in2());
    }
};

interpolate_stencil::interpolate_stencil(grid_t const &grid, double weight)
    : m_stencil(gridtools::make_computation<backend_t>(grid,
          p_weight() = gridtools::make_global_parameter<backend_t>(weight),
          gridtools::make_multistage(gridtools::execute::parallel{},
              gridtools::make_stage<interpolate_stage>(p_in1{}, p_in2(), p_weight(), p_out())))) {}

void interpolate_stencil::run(data_store_t &in1, data_store_t &in2, data_store_t &out) {
    m_stencil.run(p_in1() = in1, p_in2() = in2, p_out() = out);
}
