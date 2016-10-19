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
#include "storage/partitioner.hpp"
#include "stencil-composition/axis.hpp"
#ifdef CXX11_ENABLED
#include "../../common/generic_metafunctions/is_pack_of.hpp"
#endif

namespace gridtools {

    template < typename Axis, typename Partitioner = partitioner_dummy >
    struct grid : public clonable_to_gpu< grid< Axis, Partitioner > > {
        GRIDTOOLS_STATIC_ASSERT((is_interval< Axis >::value), "Internal Error: wrong type");
        typedef Axis axis_type;
        typedef Partitioner partitioner_t;

        typedef typename boost::mpl::plus<
            boost::mpl::minus< typename Axis::ToLevel::Splitter, typename Axis::FromLevel::Splitter >,
            static_int< 1 > >::type size_type;

        array< uint_t, size_type::value > value_list;

        GT_FUNCTION grid(const grid< Axis, Partitioner > &other)
            : m_partitioner(other.m_partitioner), m_direction_i(other.m_direction_i),
              m_direction_j(other.m_direction_j) {
            value_list = other.value_list;
        }

        GT_FUNCTION
        explicit grid(halo_descriptor const &direction_i, halo_descriptor const &direction_j)
            : m_partitioner(partitioner_dummy()), m_direction_i(direction_i), m_direction_j(direction_j) {
            GRIDTOOLS_STATIC_ASSERT(is_partitioner_dummy< partitioner_t >::value,
                "you have to construct the grid with a valid partitioner, or with no partitioner at all.");
        }

        GT_FUNCTION
        explicit grid(halo_descriptor const &direction_i,
            halo_descriptor const &direction_j,
            array< uint_t, size_type::value > const &value_list)
            : m_partitioner(partitioner_dummy()), m_direction_i(direction_i), m_direction_j(direction_j),
              value_list(value_list) {
            GRIDTOOLS_STATIC_ASSERT(is_partitioner_dummy< partitioner_t >::value,
                "you have to construct the grid with a valid partitioner, or with no partitioner at all.");
        }

        template < typename ParallelStorage >
        GT_FUNCTION explicit grid(const Partitioner &part_, ParallelStorage const &storage_)
            : m_partitioner(part_), m_direction_i(storage_.template get_halo_descriptor< 0 >()) // copy
              ,
              m_direction_j(storage_.template get_halo_descriptor< 1 >()) // copy
        {
            GRIDTOOLS_STATIC_ASSERT(!is_partitioner_dummy< Partitioner >::value,
                "you have to add the partitioner to the grid template parameters");
        }

        GT_FUNCTION
        explicit grid(uint_t *i, uint_t *j /*, uint_t* k*/)
            : m_partitioner(partitioner_dummy()) // ok since partitioner_dummy is empty. Generates a warning
              ,
              m_direction_i(i[minus], i[plus], i[begin], i[end], i[length]),
              m_direction_j(j[minus], j[plus], j[begin], j[end], j[length]) {
            GRIDTOOLS_STATIC_ASSERT(is_partitioner_dummy< partitioner_t >::value,
                "you have to construct the grid with a valid partitioner, or with no partitioner at all.");
        }

        GT_FUNCTION
        explicit grid(array< uint_t, 5 > &i, array< uint_t, 5 > &j)
            : m_partitioner(partitioner_dummy()) // ok since partitioner_dummy is empty. Generates a warning
              ,
              m_direction_i(i[minus], i[plus], i[begin], i[end], i[length]),
              m_direction_j(j[minus], j[plus], j[begin], j[end], j[length]) {
            GRIDTOOLS_STATIC_ASSERT(is_partitioner_dummy< partitioner_t >::value,
                "you have to construct the grid with a valid partitioner, or with no partitioner at all.");
        }

        GT_FUNCTION
        uint_t i_low_bound() const { return m_direction_i.begin(); }

        GT_FUNCTION
        uint_t i_high_bound() const { return m_direction_i.end(); }

        GT_FUNCTION
        uint_t j_low_bound() const { return m_direction_j.begin(); }

        GT_FUNCTION
        uint_t j_high_bound() const { return m_direction_j.end(); }

        template < typename Level >
        GT_FUNCTION uint_t value_at() const {
            GRIDTOOLS_STATIC_ASSERT((is_level< Level >::value), "Internal Error: wrong type");
            int_t offs = Level::Offset::value;
            if (offs < 0)
                offs += 1;
            return value_list[Level::Splitter::value] + offs;
        }

        GT_FUNCTION
        uint_t value_at_top() const {
            return value_list[size_type::value - 1];
            // return m_k_high_bound;
        }

        GT_FUNCTION
        uint_t value_at_bottom() const {
            return value_list[0];
            // return m_k_low_bound;
        }

        halo_descriptor const &direction_i() const { return m_direction_i; }

        halo_descriptor const &direction_j() const { return m_direction_j; }

        const Partitioner &partitioner() const {
            // the partitioner must be set
            return m_partitioner;
        }

        template < typename Flag >
        bool at_boundary(ushort_t const &coordinate_, Flag const &flag_) const {
            return m_partitioner.at_boundary(coordinate_, flag_);
        }

      private:
        Partitioner const &m_partitioner;
        halo_descriptor m_direction_i;
        halo_descriptor m_direction_j;
    };

    template < typename Grid >
    struct is_grid : boost::mpl::false_ {};

    template < typename Axis >
    struct is_grid< grid< Axis > > : boost::mpl::true_ {};

    template < typename Axis, typename Partitioner >
    struct is_grid< grid< Axis, Partitioner > > : boost::mpl::true_ {};

#ifdef CXX11_ENABLED
    template < typename... IntTypes,
        typename = is_pack_of_with_placeholder< std::is_convertible< uint_t, boost::mpl::_ >, IntTypes... > >
    GT_FUNCTION array< uint_t, sizeof...(IntTypes) > make_k_levels(IntTypes... values) {
        return array< uint_t, sizeof...(IntTypes) >{values...};
    }

    namespace _impl {
        template < size_t n_sizes >
        array< uint_t, n_sizes + 1 > interval_sizes_to_value_list(const array< uint_t, n_sizes > &sizes) {
            array< uint_t, n_sizes + 1 > value_list;

            value_list[0] = -1;
            for (uint_t i = 1; i <= n_sizes; ++i) {
                assert(sizes[i - 1] > 0);
                value_list[i] = value_list[i - 1] + sizes[i - 1];
            }
            return value_list;
        }
    }

    /**
     * @brief Pass the sizes of the k interval (do not include empty intervals). Will return an array with runtime
     * values for the splitters. We define splitters at top and bottom, i.e. one splitter more than arguments passed.
     * The axis is then defined from <first_splitter,-1> to <last_splitter,+1>.
     */
    template < typename... IntTypes,
        typename = is_pack_of_with_placeholder< std::is_convertible< uint_t, boost::mpl::_ >, IntTypes... > >
    GT_FUNCTION array< uint_t, sizeof...(IntTypes) + 1 > make_k_axis(IntTypes... values) {
        GRIDTOOLS_STATIC_ASSERT(
            (sizeof...(IntTypes) >= 1), "You need to pass at least 1 argument to define the k-axis.");

        array< uint_t, sizeof...(IntTypes) > sizes{values...};
        return _impl::interval_sizes_to_value_list(sizes);
    }

    /*
     * @brief builds a grid
     */
    template < size_t n_splitters >
    auto make_grid(halo_descriptor const &direction_i,
        halo_descriptor const &direction_j,
        array< uint_t, n_splitters > const &value_list)
        -> grid< interval< level< 0, -1 >, level< n_splitters - 1, 1 > > > {
        return grid< interval< level< 0, -1 >, level< n_splitters - 1, 1 > > >(direction_i, direction_j, value_list);
    }

    /*
     * @brief defines an interval between two splitters following the convention that each interval starts at
     * <from,+1> and ends at <to,-1>
     */
    template < uint_t from, uint_t to >
    using define_interval = interval< level< from, 1 >, level< to, -1 > >;

    /*
     * @brief make axis without halo or padding
     */
    halo_descriptor make_ij_axis(uint_t compute_interval) {
        return halo_descriptor(0, 0, 0, compute_interval - 1, compute_interval);
    }

    /*
     * @brief make axis with halo
     */
    halo_descriptor make_ij_axis(uint_t halo_minus, uint_t compute_interval, uint_t halo_plus) {
        return halo_descriptor(halo_minus,
            halo_plus,
            halo_minus,
            halo_minus + compute_interval - 1,
            halo_minus + compute_interval + halo_plus);
    }

    /*
     * @brief make axis with halo and padding
     */
    halo_descriptor make_ij_axis_padded(
        uint_t pad_minus, uint_t halo_minus, uint_t compute_interval, uint_t halo_plus, uint_t pad_plus) {
        return halo_descriptor(halo_minus,
            halo_plus,
            pad_minus + halo_minus,
            pad_minus + halo_minus + compute_interval - 1,
            pad_minus + halo_minus + compute_interval + halo_plus + pad_plus);
    }

#endif
}
