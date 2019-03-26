/*
 * GridTools
 *
 * Copyright (c) 2014-2019, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include "../../../meta.hpp"
#include "../../backend_naive/basic_token_execution_naive.hpp"
#include "../../iteration_policy.hpp"
#include "../../pos3.hpp"
#include "../esf_metafunctions.hpp"
#include "../stage.hpp"
#include "./iterate_domain_naive.hpp"
#include "./run_esf_functor_naive.hpp"

namespace gridtools {

    namespace _impl_icnaive {
        template <size_t Color>
        struct loop_interval_contains_color {
            template <class T>
            GT_META_DEFINE_ALIAS(apply,
                meta::any_of,
                (stage_group_contains_color<Color>::template apply, GT_META_CALL(meta::at_c, (T, 2))));
        };

        /**
         * @tparam RunFunctorArguments run functor argument type with the main configuration of the MSS
         * @tparam IterateDomain iterator domain class
         * @tparam Grid grid object as it provided by user.
         */
        template <typename RunFunctorArguments, typename IterateDomain, typename Grid>
        struct color_execution_functor {
          private:
            GT_STATIC_ASSERT((is_run_functor_arguments<RunFunctorArguments>::value), GT_INTERNAL_ERROR);
            GT_STATIC_ASSERT((is_iterate_domain<IterateDomain>::value), GT_INTERNAL_ERROR);
            GT_STATIC_ASSERT((is_grid<Grid>::value), GT_INTERNAL_ERROR);

            template <class Color>
            GT_META_DEFINE_ALIAS(has_color,
                meta::any_of,
                (loop_interval_contains_color<Color::value>::template apply,
                    typename RunFunctorArguments::loop_intervals_t));

            IterateDomain &m_it_domain;
            Grid const &m_grid;
            uint_t m_loop_size;

          public:
            color_execution_functor(IterateDomain &it_domain, Grid const &grid, uint_t loop_size)
                : m_it_domain(it_domain), m_grid(grid), m_loop_size(loop_size) {}

            template <class Color, enable_if_t<has_color<Color>::value, int> = 0>
            void operator()(Color) const {
                for (uint_t j = 0; j != m_loop_size; ++j) {
                    auto memorized_index = m_it_domain.index();
                    run_functors_on_interval<RunFunctorArguments, run_esf_functor_naive<Color::value>>(
                        m_it_domain, m_grid);
                    m_it_domain.set_index(memorized_index);
                    m_it_domain.increment_j();
                }
                m_it_domain.increment_j(-m_loop_size);
                m_it_domain.increment_c();
            }
            template <class Color, enable_if_t<!has_color<Color>::value, int> = 0>
            void operator()(Color) const {
                // If there is no ESF in the sequence matching the color, we skip execution and simply increment the
                // color iterator
                m_it_domain.increment_c();
            }
        };

    } // namespace _impl_icnaive

    /**
     * @brief main functor that setups the CUDA kernel for a MSS and launchs it
     * @tparam RunFunctorArguments run functor argument type with the main configuration of the MSS
     */
    template <typename RunFunctorArguments>
    struct execute_kernel_functor_naive {
        GT_STATIC_ASSERT((is_run_functor_arguments<RunFunctorArguments>::value), GT_INTERNAL_ERROR);
        typedef typename RunFunctorArguments::local_domain_t local_domain_t;
        typedef typename RunFunctorArguments::grid_t grid_t;
        typedef typename RunFunctorArguments::esf_sequence_t esf_sequence_t;

        typedef typename extract_esf_location_type<esf_sequence_t>::type location_type_t;

        static constexpr int_t n_colors = location_type_t::n_colors::value;

        typedef typename RunFunctorArguments::execution_type_t execution_type_t;

        using extent_t = GT_META_CALL(get_extent_from_loop_intervals, typename RunFunctorArguments::loop_intervals_t);

        using iterate_domain_arguments_t = iterate_domain_arguments<typename RunFunctorArguments::backend_target_t,
            local_domain_t,
            typename RunFunctorArguments::esf_sequence_t,
            typename RunFunctorArguments::cache_sequence_t,
            grid_t>;

        using iterate_domain_t = iterate_domain_naive<iterate_domain_arguments_t>;

        typedef backend_traits<target::naive> backend_traits_t;
        using interval_t = GT_META_CALL(meta::first, typename RunFunctorArguments::loop_intervals_t);
        using from_t = GT_META_CALL(meta::first, interval_t);

        execute_kernel_functor_naive(const local_domain_t &local_domain, const grid_t &grid)
            : m_local_domain(local_domain), m_grid(grid) {}

        void operator()() const {
            iterate_domain_t it_domain(m_local_domain);

            it_domain.initialize({m_grid.i_low_bound(), m_grid.j_low_bound(), m_grid.k_min()},
                {0, 0, 0},
                {extent_t::iminus::value,
                    extent_t::jminus::value,
                    static_cast<int_t>(m_grid.template value_at<from_t>() - m_grid.k_min())});

            const uint_t size_i =
                m_grid.i_high_bound() - m_grid.i_low_bound() + 1 + extent_t::iplus::value - extent_t::iminus::value;
            const uint_t size_j =
                m_grid.j_high_bound() - m_grid.j_low_bound() + 1 + extent_t::jplus::value - extent_t::jminus::value;
            for (uint_t i = 0; i != size_i; ++i) {
                gridtools::for_each<GT_META_CALL(meta::make_indices_c, n_colors)>(
                    _impl_icnaive::color_execution_functor<RunFunctorArguments, iterate_domain_t, grid_t>{
                        it_domain, m_grid, size_j});
                it_domain.increment_c(integral_constant<int, -n_colors>{});
                it_domain.increment_i();
            }
        }

      private:
        const local_domain_t &m_local_domain;
        const grid_t &m_grid;
    };
} // namespace gridtools