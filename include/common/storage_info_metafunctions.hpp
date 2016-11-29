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

#include <cmath>

#include "layout_map.hpp"
#include "nano_array.hpp"
#include "halo.hpp"

namespace gridtools {

    /* struct used to extend a given number of dimensions with a given halo */
    template < unsigned HaloVal, int LayoutArg >
    struct extend_by_halo {
        template < typename Dim >
        static constexpr unsigned extend(Dim d) {
            return (LayoutArg == -1) ? 1 : d + 2 * HaloVal;
        }
    };

    /* struct needed to calculate the aligned dimensions */
    template < typename Alignment, unsigned Length, int LayoutArg >
    constexpr unsigned align_dimensions(unsigned dimension) {
        return (Alignment::value && (LayoutArg == Length - 1))
                   ? std::ceil((float)dimension / (float)Alignment::value) * Alignment::value
                   : dimension;
    }

    template < typename Layout, typename Alignment, typename Halo >
    struct get_initial_offset;

    template < int... LayoutArgs, typename Alignment, unsigned... HaloVals >
    struct get_initial_offset< layout_map< LayoutArgs... >, Alignment, halo< HaloVals... > > {
        constexpr static unsigned compute() {
            return (Alignment::value &&
                       get_value_from_pack(get_index_of_element_in_pack(
                                               0, (layout_map< LayoutArgs... >::unmasked_length - 1), LayoutArgs...),
                           HaloVals...))
                       ? (std::ceil((float)get_value_from_pack(
                                        get_index_of_element_in_pack(
                                            0, (layout_map< LayoutArgs... >::unmasked_length - 1), LayoutArgs...),
                                        HaloVals...) /
                                    (float)Alignment::value) *
                                 Alignment::value -
                             get_value_from_pack(
                                 get_index_of_element_in_pack(
                                     0, (layout_map< LayoutArgs... >::unmasked_length - 1), LayoutArgs...),
                                 HaloVals...))
                       : 0;
        }
    };

    /* struct needed to calculate the strides */
    template < typename T >
    struct get_strides_aux;

    template < int... LayoutArgs >
    struct get_strides_aux< layout_map< LayoutArgs... > > {
        template < typename T, typename... Dims >
        static constexpr unsigned get_stride(T N, Dims... d) {
            return (N == -1)
                       ? 0
                       : ((N == (layout_map< LayoutArgs... >::unmasked_length - 1))
                                 ? 1
                                 : (get_value_from_pack(get_index_of_element_in_pack(0, N + 1, LayoutArgs...), d...)) *
                                       get_stride(N + 1, d...));
        }
    };

    template < typename Layout >
    struct get_strides;

    template < int... LayoutArgs >
    struct get_strides< layout_map< LayoutArgs... > > {
        template < typename... Dims >
        static constexpr nano_array< unsigned, sizeof...(LayoutArgs) > get_stride_array(Dims... d) {
            typedef layout_map< LayoutArgs... > Layout;
            return (nano_array< unsigned, sizeof...(LayoutArgs) >){
                get_strides_aux< Layout >::template get_stride(LayoutArgs, d...)...};
        }
    };
}
