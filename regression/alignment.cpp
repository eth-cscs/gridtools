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

#include <gtest/gtest.h>

#include <gridtools/stencil-composition/stencil-composition.hpp>
#include <gridtools/tools/regression_fixture.hpp>

/**
  @file
  This file shows an implementation of the "copy" stencil, simple copy of one field done on the backend,
  in which a misaligned storage is aligned
*/

using namespace gridtools;
using namespace enumtype;

// These are the stencil operators that compose the multistage stencil in this test
struct copy_functor {
    using in = in_accessor<0>;
    using out = inout_accessor<1>;
    using arg_list = boost::mpl::vector<in, out>;

#ifdef __CUDACC__
    /** @brief checking all storages alignment using a specific storage_info

        \tparam Index index of the storage which alignment should be checked
        \tparam ItDomain iterate domain type
        \param it_domain iterate domain, used to get the pointers and offsets
        \param alignment ordinal number identifying the alignment
    */
    template <typename Ptr>
    GT_FUNCTION static bool check_pointer_alignment(Ptr const *ptr, uint_t alignment) {
        return threadIdx.x != 0 && (uintptr_t)ptr % alignment == 0;
    }
#endif

    template <typename Evaluation>
    GT_FUNCTION static void Do(Evaluation &eval) {

#ifdef __CUDACC__
#ifndef NDEBUG
        if (!check_pointer_alignment(&eval(in()), sizeof(float_type) * meta_data_t::alignment_t::value) ||
            !check_pointer_alignment(&eval(out()), sizeof(float_type) * meta_data_t::alignment_t::value)) {
            printf("alignment error in some storages with first meta_storage \n");
            assert(false);
        }
#endif
#endif
        eval(out()) = eval(in());
    }
};

using AlignedCopyStencil = regression_fixture<2>;

TEST_F(AlignedCopyStencil, Test) {
    using halo_t = halo<2, 2, 2>;
    using meta_data_t = backend_t::storage_traits_t::storage_info_t<0, 3, halo_t>;
    using storage_t = backend_t::storage_traits_t::data_store_t<float_type, meta_data_t>;

    meta_data_t meta_data_(d1() + 2 * halo_t::at<0>(), d2() + 2 * halo_t::at<1>(), d3() + 2 * halo_t::at<2>());

    halo_descriptor di{halo_t::at<0>(), 0, halo_t::at<0>(), d1() + halo_t::at<0>() - 1, d1() + halo_t::at<0>()};
    halo_descriptor dj{halo_t::at<1>(), 0, halo_t::at<1>(), d2() + halo_t::at<1>() - 1, d2() + halo_t::at<1>()};
    grid<axis<1>::axis_interval_t> grid(di, dj, {halo_t::at<2>(), d3() + halo_t::at<2>()});

    // Definition of the actual data fields that are used for input/output
    storage_t out(meta_data_, -1.);
    storage_t in(meta_data_, [](int i, int j, int k) { return i + j + k; });

    arg<0, storage_t> p_in;
    arg<1, storage_t> p_out;

    gridtools::make_positional_computation<backend_t>(grid,
        p_in = in,
        p_out = out,
        make_multistage(enumtype::execute<enumtype::forward>(), make_stage<copy_functor>(p_in, p_out)))
        .run();
    verify(in, out);
}
