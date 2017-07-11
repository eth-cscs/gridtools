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
#include "../host_device.hpp"

namespace gridtools {

    /**@brief operation to be used inside the accumulator*/
    struct logical_and {
        GT_FUNCTION
        constexpr logical_and() {}
        template < typename T >
        GT_FUNCTION constexpr T operator()(const T &x, const T &y) const {
            return x && y;
        }
    };

    /**@brief operation to be used inside the accumulator*/
    struct logical_or {
        GT_FUNCTION
        constexpr logical_or() {}
        template < typename T >
        GT_FUNCTION constexpr T operator()(const T &x, const T &y) const {
            return x || y;
        }
    };

    /**@brief binary operator functor that checks if two types passed fulfill the == operator*/
    struct equal {
        GT_FUNCTION
        constexpr equal() {}

        template < typename T >
        GT_FUNCTION constexpr bool operator()(const T &x, const T &y) const {
            return x == y;
        }
    };
}
