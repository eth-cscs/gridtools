/*
  GridTools Libraries

  Copyright (c) 2016, GridTools Consortium
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
#include "host_device.hpp"

namespace gridtools {

    /**@brief Class in substitution of std::pow, not available in CUDA*/
    template < uint_t Number >
    struct gt_pow {
        template < typename Value >
        GT_FUNCTION static Value constexpr apply(Value const &v) {
            return v * gt_pow< Number - 1 >::apply(v);
        }
    };

    /**@brief Class in substitution of std::pow, not available in CUDA*/
    template <>
    struct gt_pow< 0 > {
        template < typename Value >
        GT_FUNCTION static Value constexpr apply(Value const &v) {
            return 1.;
        }
    };

    namespace math {

        template < typename Value >
        GT_FUNCTION Value max(Value const val0) {
            assert(false);
        }

        template < typename Value >
        GT_FUNCTION Value max() {
            assert(false);
        }

        template < typename Value >
        GT_FUNCTION Value max(Value const val0, Value const val1) {
            return val0 > val1 ? val0 : val1;
        }

        template < typename ValueFirst, typename... OtherValues >
        GT_FUNCTION ValueFirst max(ValueFirst val0, OtherValues... vals) {
            return val0 > max(vals...) ? val0 : max(vals...);
        }

        template < typename Value >
        GT_FUNCTION Value min(Value const val0) {
            assert(false);
        }

        template < typename Value >
        GT_FUNCTION Value min() {
            assert(false);
        }

        template < typename Value >
        GT_FUNCTION Value min(Value const val0, Value const val1) {
            return val0 > val1 ? val1 : val0;
        }

        template < typename ValueFirst, typename... OtherValues >
        GT_FUNCTION ValueFirst min(ValueFirst val0, OtherValues... vals) {
            return val0 > min(vals...) ? min(vals...) : val0;
        }

        template < typename Value >
        GT_FUNCTION Value fabs(Value const val) {
            return ::fabs(val);
        }
        template < typename Value >
        GT_FUNCTION Value abs(Value const val) {
            return ::abs(val);
        }
    } // namespace math

} // namespace gridtools
