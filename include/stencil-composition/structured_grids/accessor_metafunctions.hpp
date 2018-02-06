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

#include "accessor_fwd.hpp"
#include "./accessor.hpp"
#include "../expressions/expressions.hpp"

namespace gridtools {

    template < typename T >
    struct is_regular_accessor : boost::mpl::false_ {};

    template < uint_t ID, enumtype::intent Intent, typename Extent, ushort_t Number >
    struct is_regular_accessor< accessor< ID, Intent, Extent, Number > > : boost::mpl::true_ {};

    template < uint_t ID, enumtype::intent Intent, typename Extent, ushort_t Number >
    struct is_regular_accessor< accessor_base< ID, Intent, Extent, Number > > : boost::mpl::true_ {};

    template < typename T >
    struct is_regular_accessor< const T > : is_regular_accessor< T > {};

    template < typename T >
    struct is_accessor : boost::mpl::false_ {};

    template < uint_t ID, enumtype::intent Intent, typename Extent, ushort_t Number >
    struct is_accessor< accessor< ID, Intent, Extent, Number > > : boost::mpl::true_ {};

    template < uint_t ID, enumtype::intent Intent, typename Extent, ushort_t Number >
    struct is_accessor< accessor_base< ID, Intent, Extent, Number > > : boost::mpl::true_ {};

    template < typename T >
    struct is_grid_accessor : boost::mpl::false_ {};

    template < uint_t ID, enumtype::intent Intent, typename Extent, ushort_t Number >
    struct is_grid_accessor< accessor< ID, Intent, Extent, Number > > : boost::mpl::true_ {};

    template < uint_t ID, enumtype::intent Intent, typename Extent, ushort_t Number >
    struct is_grid_accessor< accessor_base< ID, Intent, Extent, Number > > : boost::mpl::true_ {};

    // TODO add documentation
    template < typename Accessor, unsigned Ext >
    struct accessor_extend;

    template < ushort_t ID, enumtype::intent Intent, typename Extent, ushort_t Number, unsigned Ext >
    struct accessor_extend< accessor< ID, Intent, Extent, Number >, Ext > {
        typedef accessor< ID, Intent, Extent, (Number + Ext) > type;
    };

    /**
     * @brief metafunction that given an accesor and a map, it will remap the index of the accessor according
     * to the corresponding entry in ArgsMap
     */
    template < typename Accessor, typename ArgsMap, typename Enable = void >
    struct remap_accessor_type {};

    // TODO(havogt) I have no idea why I end up here...
    template < typename ArgsMap >
    struct remap_accessor_type< boost::mpl::void_, ArgsMap > {
        using type = boost::mpl::void_;
    };

    template < ushort_t ID, enumtype::intent Intent, typename Extent, ushort_t Number, typename ArgsMap >
    struct remap_accessor_type< accessor< ID, Intent, Extent, Number >, ArgsMap > {
        using type = accessor< _impl::get_remap_accessor_id< ID, ArgsMap >(), Intent, Extent, Number >;
    };

    template < typename ArgsMap, template < typename... > class Expression, typename... Arguments >
    struct remap_accessor_type< Expression< Arguments... >,
        ArgsMap,
        typename boost::enable_if< typename is_expr< Expression< Arguments... > >::type, void >::type > {
        // Expression is an expression of accessors (e.g. expr_sum<T1, T2>,
        // where T1 and T2 are two accessors).
        // Here we traverse the expression AST down to the leaves, and we assert if
        // the leaves are not accessor types.

        // recursively remapping the template arguments,
        // until the specialization above stops the recursion
        typedef Expression< typename remap_accessor_type< Arguments, ArgsMap >::type... > type;
    };

    template < typename T, typename ArgsMap >
    struct remap_accessor_type< T,
        ArgsMap,
        typename boost::enable_if< typename boost::is_arithmetic< T >::type, void >::type > {
        // when a leaf don't do anything
        typedef T type;
    };

    template < typename ArgsMap, template < typename Acc, int N > class Expression, typename Accessor, int Number >
    struct remap_accessor_type< Expression< Accessor, Number >, ArgsMap > {
        // Specialization done to catch also the "pow" expression, for which a template argument is an
        // integer (the exponent)
        typedef Expression< typename remap_accessor_type< Accessor, ArgsMap >::type, Number > type;
    };
} // namespace gridtools
