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

#include "../../meta.hpp"
#include "../builtins.hpp"
#include "generate.hpp"
#include "nodes.hpp"
#include "parse.hpp"

namespace gridtools::fn::ast {
    namespace tmp_impl_ {
        namespace lazy {
            template <class Leaf>
            struct normailze_shifts {
                using type = Leaf;
            };

            template <template <class...> class Node, class... Trees>
            struct normailze_shifts<Node<Trees...>> {
                using type = Node<typename normailze_shifts<Trees>::type...>;
            };

            template <class Tree, auto... Outer, auto... Inner>
            struct normailze_shifts<shifted<shifted<Tree, meta::val<Inner...>>, meta::val<Outer...>>> {
                using type = typename normailze_shifts<shifted<Tree, meta::val<Inner..., Outer...>>>::type;
            };
        } // namespace lazy
        GT_META_DELEGATE_TO_LAZY(normailze_shifts, class T, T);

        template <class>
        struct has_tmps : std::false_type {};

        template <template <class...> class Node, class... Trees>
        struct has_tmps<Node<Trees...>> : std::disjunction<has_tmps<Trees>...> {};

        template <auto F, class... Trees>
        struct has_tmps<lambda<meta::val<F>, Trees...>> : has_tmps<decltype(F(Trees()...))> {};

        template <class F, class... Trees>
        struct has_tmps<inlined<F, Trees...>> : has_tmps<lambda<F, Trees...>> {};

        template <class F, class... Trees>
        struct has_tmps<tmp<F, Trees...>> : std::true_type {};

        template <class T>
        using is_arg =
            std::bool_constant<meta::is_instantiation_of<tmp, T>::value || meta::is_instantiation_of<in, T>::value>;

        template <class>
        struct collect_args {
            using type = meta::list<>;
        };

        template <template <class...> class Node, class... Trees>
        struct collect_args<Node<Trees...>> {
            using type = meta::dedup<meta::concat<typename collect_args<Trees>::type...>>;
        };

        template <class V>
        struct collect_args<in<V>> {
            using type = meta::list<in<V>>;
        };

        template <class F, class... Args>
        struct collect_args<tmp<F, Args...>> {
            using type = meta::list<tmp<F, Args...>>;
        };

        template <class...>
        struct remap_args;

        template <class Map, class Leaf>
        struct remap_args<Map, Leaf> {
            using type = Leaf;
        };

        template <class Map, template <class...> class Node, class... Trees>
        struct remap_args<Map, Node<Trees...>> {
            using type = Node<typename remap_args<Map, Trees>::type...>;
        };

        template <class Map, class V>
        struct remap_args<Map, in<V>> {
            using type = in<meta::second<meta::mp_find<Map, in<V>>>>;
        };

        template <class Map, class F, class... Args>
        struct remap_args<Map, tmp<F, Args...>> {
            using type = in<meta::second<meta::mp_find<Map, tmp<F, Args...>>>>;
        };

        template <class...>
        struct collapse;

        template <template <class...> class Node, class Tree>
        struct collapse_impl {
            using old_args_t = typename collect_args<Tree>::type;
            using map_t = meta::zip<old_args_t, meta::make_indices_for<old_args_t>>;
            using fun_t = meta::val<generate<typename remap_args<map_t, Tree>::type>>;
            using args_t = meta::transform<meta::force<collapse>::apply, old_args_t>;
            using type = meta::rename<Node, meta::push_front<args_t, fun_t>>;
        };

        template <class Tree>
        struct collapse<Tree> {
            using type = typename meta::if_<has_tmps<Tree>, collapse_impl<lambda, Tree>, meta::lazy::id<Tree>>::type;
        };

        template <class F, class... Trees>
        struct collapse<lambda<F, Trees...>> {
            using type = meta::if_<std::disjunction<is_arg<Trees>...>,
                lambda<F, typename collapse<Trees>::type...>,
                typename collapse_impl<lambda, lambda<F, Trees...>>::type>;
        };

        template <class F, class... Trees>
        struct collapse<tmp<F, Trees...>> {
            using type = meta::if_<std::disjunction<is_arg<Trees>...>,
                tmp<F, typename collapse<Trees>::type...>,
                typename collapse_impl<tmp, lambda<F, Trees...>>::type>;
        };

        template <class T>
        struct expand {
            using type = T;
        };

        template <template <class...> class Node, class... Trees>
        struct expand<Node<Trees...>> {
            using type = Node<typename expand<Trees>::type...>;
        };

        template <class F, class... Trees>
        struct expand<lambda<F, Trees...>> {
            using type = meta::if_<has_tmps<parse<F::value, Trees...>>,
                typename expand<normailze_shifts<decltype(F::value(Trees()...))>>::type,
                lambda<F, typename expand<Trees>::type...>>;
        };

        template <class F, class... Trees>
        struct expand<tmp<F, Trees...>> {
            using type = meta::if_<has_tmps<parse<F::value, Trees...>>,
                meta::rename<tmp, typename collapse<typename expand<lambda<F, Trees...>>::type>::type>,
                tmp<F, typename expand<Trees>::type...>>;
        };

        template <class F, class... Trees>
        struct expand<deref<inlined<F, Trees...>>> {
            using type = typename expand<lambda<F, Trees...>>::type;
        };

        template <class Offsets, class F, class... Trees>
        struct expand<deref<shifted<inlined<F, Trees...>, Offsets>>> {
            using type = typename expand<lambda<F, shifted<Trees, Offsets>...>>::type;
        };

        template <class I, class F, class... Trees>
        struct expand<deref<tuple_get<I, inlined<F, Trees...>>>> {
            using type = typename expand<tuple_get<I, lambda<F, Trees...>>>::type;
        };

        template <class I, class Offsets, class F, class... Trees>
        struct expand<deref<shifted<tuple_get<I, inlined<F, Trees...>>, Offsets>>> {
            using type = typename expand<tuple_get<I, lambda<F, shifted<Trees, Offsets>...>>>::type;
        };

        template <class Tree>
        using popup_tmps = typename collapse<typename expand<normailze_shifts<Tree>>::type>::type;

        template <class...>
        struct flatten_nodes;

        template <class V>
        struct flatten_nodes<in<V>> {
            using type = meta::list<>;
        };

        template <class F, class... Trees>
        struct flatten_nodes<lambda<F, Trees...>> {
            using type =
                meta::dedup<meta::push_back<meta::concat<typename flatten_nodes<Trees>::type...>, lambda<F, Trees...>>>;
        };

        template <class F, class... Trees>
        struct flatten_nodes<tmp<F, Trees...>> {
            using type =
                meta::dedup<meta::push_back<meta::concat<typename flatten_nodes<Trees>::type...>, tmp<F, Trees...>>>;
        };

        template <class...>
        struct collect_offsets;

        template <class Tmp, class Tree>
        struct collect_offsets<Tmp, Tree> {
            using type = meta::list<>;
        };

        template <class Tmp, template <class...> class Node, class... Trees>
        struct collect_offsets<Tmp, Node<Trees...>> {
            using type = meta::dedup<meta::concat<typename collect_offsets<Tmp, Trees>::type...>>;
        };

        template <class Tmp, auto F, class... Trees>
        struct collect_offsets<Tmp, lambda<meta::val<F>, Trees...>> {
            using type = typename collect_offsets<Tmp, normailze_shifts<decltype(F(Trees()...))>>::type;
        };

        template <class Tmp>
        struct collect_offsets<Tmp, deref<Tmp>> {
            using type = meta::list<meta::val<>>;
        };

        template <class Tmp, class Offsets>
        struct collect_offsets<Tmp, deref<shifted<Tmp, Offsets>>> {
            using type = meta::list<Offsets>;
        };

        template <class Tmp, class F, class... Trees>
        struct collect_offsets<Tmp, deref<inlined<F, Trees...>>> {
            using type = typename collect_offsets<Tmp, lambda<F, Trees...>>::type;
        };

        template <class Tmp, class Offsets, class F, class... Trees>
        struct collect_offsets<Tmp, deref<shifted<inlined<F, Trees...>, Offsets>>> {
            using type = typename collect_offsets<Tmp, lambda<F, shifted<Trees, Offsets>...>>::type;
        };

        template <class I, class Tmp, class F, class... Trees>
        struct collect_offsets<Tmp, deref<tuple_get<I, inlined<F, Trees...>>>> {
            using type = typename collect_offsets<Tmp, lambda<F, Trees...>>::type;
        };

        template <class I, class Tmp, class Offsets, class F, class... Trees>
        struct collect_offsets<Tmp, deref<shifted<tuple_get<I, inlined<F, Trees...>>, Offsets>>> {
            using type = typename collect_offsets<Tmp, lambda<F, shifted<Trees, Offsets>...>>::type;
        };

        template <class T>
        struct dummy_iter {
            friend T fn_builtin(builtins::deref, dummy_iter) { return {}; }
            friend dummy_iter fn_builtin(builtins::shift, auto &&, dummy_iter) { return {}; }
        };

        template <class Map>
        struct make_dummy_iter_f {
            template <class Arg>
            using apply = dummy_iter<meta::third<meta::mp_find<Map, Arg>>>;
        };

        template <class Map>
        struct arg_num_f {
            template <class Arg>
            using apply = meta::second<meta::mp_find<Map, Arg>>;
        };

        template <class...>
        struct get_args;

        template <class F, class... Args>
        struct get_args<lambda<F, Args...>> : meta::list<Args...> {};

        template <class F, class... Args>
        struct get_args<tmp<F, Args...>> : meta::list<Args...> {};

        template <class...>
        struct get_fun;

        template <class F, class... Args>
        struct get_fun<lambda<F, Args...>> {
            using type = F;
        };

        template <class F, class... Args>
        struct get_fun<tmp<F, Args...>> {
            using type = F;
        };

        template <class Map, class Tree>
        struct make_tmp_record_f {
            template <class Tmp, class Item = meta::mp_find<Map, Tmp>>
            using apply = meta::list<meta::third<Item>,
                typename collect_offsets<Tmp, Tree>::type,
                typename get_fun<Tmp>::type,
                meta::list<meta::second<Item>>,
                meta::transform<arg_num_f<Map>::template apply, typename get_args<Tmp>::type>>;
        };

        template <class...>
        struct ret_type;

        template <class Map, class F, class... Args>
        struct ret_type<Map, lambda<F, Args...>> {
            using type = decltype(F::value(dummy_iter<meta::third<meta::mp_find<Map, Args>>>()...));
        };

        template <class Map, class F, class... Args>
        struct ret_type<Map, tmp<F, Args...>> {
            using type = decltype(F::value(dummy_iter<meta::third<meta::mp_find<Map, Args>>>()...));
        };

        template <class Map, class Tmp>
        using add_to_arg_map =
            meta::push_back<Map, meta::list<Tmp, typename meta::length<Map>::type, typename ret_type<Map, Tmp>::type>>;

        template <class V>
        struct add_f {
            template <class T>
            using apply = std::integral_constant<typename T::value_type, V::value + T::value>;
        };

        template <class Tree,
            class InputTypes,
            class Nodes = typename flatten_nodes<Tree>::type,
            class InitialArgMap = meta::zip<meta::transform<in, meta::make_indices_for<InputTypes>>,
                meta::make_indices_for<InputTypes>,
                InputTypes>,
            class ArgMap = meta::foldl<add_to_arg_map, InitialArgMap, Nodes>>
        using flatten_tmps_tree = meta::transform<make_tmp_record_f<ArgMap, Tree>::template apply, Nodes>;

    } // namespace tmp_impl_

    template <auto F, class... Args>
    constexpr bool has_tmps = tmp_impl_::has_tmps<parse<F, Args...>>::value;

    using tmp_impl_::flatten_tmps_tree;
    using tmp_impl_::popup_tmps;
} // namespace gridtools::fn::ast