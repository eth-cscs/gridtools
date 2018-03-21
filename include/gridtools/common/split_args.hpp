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

#include <utility>
#include <tuple>
#include <type_traits>

#include "defs.hpp"

#if GT_BROKEN_TEMPLATE_ALIASES
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/include/std_tuple.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/zip_view.hpp>
#include <boost/mpl/filter_view.hpp>
#include <boost/mpl/transform_view.hpp>

#include "generic_metafunctions/copy_into_variadic.hpp"
#include "generic_metafunctions/type_traits.hpp"
#else
#include "generic_metafunctions/meta.hpp"
#endif

namespace gridtools {
    namespace _impl {
        namespace _split_args {
#if GT_BROKEN_TEMPLATE_ALIASES

            template < template < class... > class Pred >
            struct apply_to_first {
                template < class L >
                struct apply : Pred< typename boost::mpl::at_c< L, 0 >::type > {};
            };

            template < template < class... > class Pred >
            struct apply_to_decayed {
                template < class T >
                struct apply : Pred< typename std::decay< T >::type > {};
            };

            template < template < class... > class Pred, class Args >
            using make_filtered_indicies = copy_into_variadic<
                boost::mpl::transform_view<
                    boost::mpl::filter_view< boost::mpl::zip_view< boost::mpl::vector< Args,
                                                 boost::mpl::range_c< size_t, 0, boost::mpl::size< Args >::value > > >,
                        apply_to_first< Pred > >,
                    boost::mpl::at< boost::mpl::_, boost::mpl::size_t< 1 > > >,
                std::tuple<> >;

            template < template < class... > class Pred >
            struct not_ {
                template < class T >
                struct apply : negation< Pred< T > > {};
            };

#else
            template < template < class... > class Pred >
            struct apply_to_first {
                template < class L >
                using apply = Pred< meta::first< L > >;
            };

            template < template < class... > class Pred >
            struct apply_to_decayed {
                template < class T >
                using apply = Pred< typename std::decay< T >::type >;
            };

            template < template < class... > class Pred, class Args >
            using make_filtered_indicies = meta::apply< meta::transform< meta::second >,
                meta::apply< meta::filter< apply_to_first< Pred >::template apply >,
                                                            meta::zip< Args, meta::make_indices_for< Args > > > >;
            using meta::not_;
#endif
            template < class Args, template < class... > class L, class... Is >
            auto get_part_helper(Args &&args, L< Is... > *)
                GT_AUTO_RETURN(std::forward_as_tuple(std::get< Is::value >(std::forward< Args >(args))...));

            template < template < class... > class Pred, class Args >
            auto get_part(Args &&args) GT_AUTO_RETURN(
                get_part_helper(std::forward< Args >(args), (make_filtered_indicies< Pred, Args > *)(nullptr)));

            template < template < class... > class Pred, class Args >
            auto raw_split_args_tuple(Args &&args)
                GT_AUTO_RETURN(std::make_pair(get_part< Pred >(std::forward< Args >(args)),
                    get_part< not_< Pred >::template apply >(std::forward< Args >(args))));

            template < template < class... > class Pred, class Args >
            auto split_args_tuple(Args &&args) GT_AUTO_RETURN(
                raw_split_args_tuple< apply_to_decayed< Pred >::template apply >(std::forward< Args >(args)));
        }
    }

    /// Variations that take a tuple instead of parameter pack
    using _impl::_split_args::raw_split_args_tuple;
    using _impl::_split_args::split_args_tuple;

    /**
     *  Split the args into two groups according to the given compile time predicate on the argument type
     *  Argument types are taken raw into predicate . With references and const modifiers.
     *
     * @return std::pair of two std::tuples. First tuple is from the types that satisfies predicate Pred
     */
    template < template < class... > class Pred, class... Args >
    auto raw_split_args(Args &&... args)
        GT_AUTO_RETURN(raw_split_args_tuple< Pred >(std::forward_as_tuple(std::forward< Args >(args)...)));

    /// A handy variation of raw_split_args that applies predicate on decayed argument types.
    template < template < class... > class Pred, class... Args >
    auto split_args(Args &&... args)
        GT_AUTO_RETURN(split_args_tuple< Pred >(std::forward_as_tuple(std::forward< Args >(args)...)));
}
