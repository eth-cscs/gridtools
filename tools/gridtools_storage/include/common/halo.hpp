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

#include <array>

#include <boost/mpl/bool.hpp>
#include <boost/mpl/accumulate.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/plus.hpp>

namespace gridtools {

    template < unsigned... N >
    struct halo {
        static constexpr unsigned value[sizeof...(N)] = {N...};

        template < unsigned V >
        static constexpr unsigned at() {
            static_assert((V < sizeof...(N)), "Out of bounds access in halo type discovered.");
            return value[V];
        }

        static constexpr unsigned at(unsigned V) { return value[V]; }

        static constexpr unsigned size() { return sizeof...(N); }
    };

    /* used to generate a zero initialzed halo. Used as a default value for storage info halo. */
    template < unsigned Cnt, unsigned... Vals >
    struct zero_halo : zero_halo< Cnt - 1, 0, Vals... > {};

    template < unsigned... Vals >
    struct zero_halo< 0, Vals... > {
        typedef typename boost::mpl::accumulate< boost::mpl::vector< boost::mpl::int_< Vals >... >,
            boost::mpl::int_< 0 >,
            boost::mpl::plus< boost::mpl::_1, boost::mpl::_2 > >::type sum;
        static_assert((sum::value == 0), "Failed to create a zero halo type");
        typedef halo< Vals... > type;
    };

    /* used to check if a given type is a halo type */
    template < typename T >
    struct is_halo : boost::mpl::false_ {};

    template < unsigned... N >
    struct is_halo< halo< N... > > : boost::mpl::true_ {};
}
