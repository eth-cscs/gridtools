/*
 * GridTools
 *
 * Copyright (c) 2014-2021, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "../../../../common/host_device.hpp"
#include "expr_base.hpp"

namespace gridtools {
    namespace stencil {
        namespace cartesian {
            namespace expressions {
                struct plus_f {
                    template <class Lhs, class Rhs>
                    GT_FORCE_INLINE constexpr auto operator()(Lhs const &lhs, Rhs const &rhs) const {
                        return lhs + rhs;
                    }
                    template <class Arg>
                    GT_FORCE_INLINE constexpr auto operator()(Arg const &arg) const {
                        return +arg;
                    }
                };

                template <class Lhs, class Rhs>
                GT_FORCE_INLINE constexpr auto operator+(Lhs lhs, Rhs rhs)
                    -> decltype(make_expr(plus_f(), Lhs(), Rhs())) {
                    return make_expr(plus_f(), lhs, rhs);
                }

                template <class Arg>
                GT_FORCE_INLINE constexpr auto operator+(Arg arg) -> decltype(make_expr(plus_f(), Arg())) {
                    return make_expr(plus_f(), arg);
                }
            } // namespace expressions
        }     // namespace cartesian
    }         // namespace stencil
} // namespace gridtools
