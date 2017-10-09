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
#include "gtest/gtest.h"
#include <stencil-composition/stencil-composition.hpp>

#include <gridtools.hpp>
#ifdef CUDA_EXAMPLE
#include <stencil-composition/backend_cuda/backend_cuda.hpp>
#else
#include <stencil-composition/backend_host/backend_host.hpp>
#endif

#include <boost/timer/timer.hpp>
#include <boost/fusion/include/make_vector.hpp>

#ifdef USE_PAPI_WRAP
#include <papi_wrap.hpp>
#include <papi.hpp>
#endif

/*
  This file shows an implementation of the "copy" stencil, simple copy of one field done on the backend
*/

using gridtools::level;
using gridtools::accessor;
using gridtools::extent;
using gridtools::arg;

using namespace gridtools;
using namespace enumtype;

namespace copy_stencils_3D_2D_1D_0D {
    // This is the definition of the special regions in the "vertical" direction
    typedef gridtools::interval< level< 0, -1 >, level< 1, -1 > > x_interval;
    typedef gridtools::interval< level< 0, -2 >, level< 1, 1 > > axis;

    // These are the stencil operators that compose the multistage stencil in this test
    struct copy_functor {
        static const int n_args = 2;
        typedef accessor< 0 > in;
        typedef accessor< 1, enumtype::inout > out;
        typedef boost::mpl::vector< in, out > arg_list;

        template < typename Evaluation >
        GT_FUNCTION static void Do(Evaluation &eval, x_interval) {
            eval(out()) = eval(in());
        }
    };

    /*
     * The following operators and structs are for debugging only
     */
    std::ostream &operator<<(std::ostream &s, copy_functor const) { return s << "copy_functor"; }

    void handle_error(int) { std::cout << "error" << std::endl; }

    template < typename SrcLayout, typename DstLayout, typename T >
    bool test(int x, int y, int z) {

        uint_t d1 = x;
        uint_t d2 = y;
        uint_t d3 = z;

#ifdef CUDA_EXAMPLE
#define BACKEND backend< Cuda, GRIDBACKEND, Block >
        typedef gridtools::cuda_storage_info< 0, DstLayout > meta_dst_t;
        typedef gridtools::cuda_storage_info< 0, SrcLayout > meta_src_t;
#else
#ifdef BACKEND_BLOCK
#define BACKEND backend< Host, GRIDBACKEND, Block >
#else
#define BACKEND backend< Host, GRIDBACKEND, Naive >
#endif
        typedef gridtools::host_storage_info< 0, DstLayout > meta_dst_t;
        typedef gridtools::host_storage_info< 0, SrcLayout > meta_src_t;
#endif

        typedef BACKEND::storage_traits_t::data_store_t< T, meta_dst_t > storage_t;
        typedef BACKEND::storage_traits_t::data_store_t< T, meta_src_t > src_storage_t;

        meta_dst_t meta_dst_(d1, d2, d3);
        meta_src_t meta_src_(d1, d2, d3);
        // Definition of the actual data fields that are used for input/output
        std::function< T(int, int, int)> init_lambda = [](int i, int j, int k) { return (T)(i + j + k); };
        src_storage_t in(meta_src_, init_lambda);
        storage_t out(meta_dst_, (T)1.5);

        // in.print();

        // Definition of placeholders. The order of them reflect the order the user will deal with them
        // especially the non-temporary ones, in the construction of the domain
        typedef arg< 0, src_storage_t > p_in;
        typedef arg< 1, storage_t > p_out;

        // An array of placeholders to be passed to the domain
        // I'm using mpl::vector, but the final API should look slightly simpler
        typedef boost::mpl::vector< p_in, p_out > accessor_list;

        // construction of the domain. The domain is the physical domain of the problem, with all the physical fields
        // that are used, temporary and not
        // It must be noted that the only fields to be passed to the constructor are the non-temporary.
        // The order in which they have to be passed is the order in which they appear scanning the placeholders in
        // order. (I don't particularly like this)
        gridtools::aggregator_type< accessor_list > domain(in, out);

        // Definition of the physical dimensions of the problem.
        // The constructor takes the horizontal plane dimensions,
        // while the vertical ones are set according the the axis property soon after
        // gridtools::grid<axis> grid_(??2,d1-2,2,d2-2??);
        halo_descriptor di{0, 0, 0, d1 - 1, d1};
        halo_descriptor dj{0, 0, 0, d2 - 1, d2};

        gridtools::grid< axis > grid_(di, dj);
        grid_.value_list[0] = 0;
        grid_.value_list[1] = d3 - 1;

        auto copy = gridtools::make_computation< gridtools::BACKEND >(domain,
            grid_,
            gridtools::make_multistage                                                    // mss_descriptor
            (execute< forward >(), gridtools::make_stage< copy_functor >(p_in(), p_out()) // esf_descriptor
                                                                          ));

        copy->ready();
        copy->steady();

        copy->run();

        copy->finalize();

        bool ok = true;
        auto outv = make_host_view(out);
        auto inv = make_host_view(in);
        for (int i = 0; i < d1; ++i)
            for (int j = 0; j < d2; ++j)
                for (int k = 0; k < d3; ++k) {
                    if (inv(i, j, k) != outv(i, j, k)) {
                        // std::cout << "i = " << i
                        //           << "j = " << j
                        //           << "k = " << k
                        //           << ": " << in(i,j,k) << ", " << out(i,j,k) << std::endl;
                        ok = false;
                    }
                }

        return ok;
    }

} // namespace copy_stencils_3D_2D_1D_0D

TEST(TESTCLASS, copies3D) {
    EXPECT_EQ(
        (copy_stencils_3D_2D_1D_0D::test< gridtools::layout_map< 0, 1, 2 >, gridtools::layout_map< 0, 1, 2 >, double >(
            __Size0, __Size1, __Size2)),
        true);
}

TEST(TESTCLASS, copies3Dtranspose) {
    EXPECT_EQ(
        (copy_stencils_3D_2D_1D_0D::test< gridtools::layout_map< 2, 1, 0 >, gridtools::layout_map< 0, 1, 2 >, double >(
            __Size0, __Size1, __Size2)),
        true);
}

TEST(TESTCLASS, copies2Dij) {
    EXPECT_EQ(
        (copy_stencils_3D_2D_1D_0D::test< gridtools::layout_map< 0, 1, -1 >, gridtools::layout_map< 0, 1, 2 >, double >(
            __Size0, __Size1, __Size2)),
        true);
}

TEST(TESTCLASS, copies2Dik) {
    EXPECT_EQ(
        (copy_stencils_3D_2D_1D_0D::test< gridtools::layout_map< 0, -1, 1 >, gridtools::layout_map< 0, 1, 2 >, double >(
            __Size0, __Size1, __Size2)),
        true);
}

TEST(TESTCLASS, copies2Djk) {
    EXPECT_EQ(
        (copy_stencils_3D_2D_1D_0D::test< gridtools::layout_map< -1, 0, 1 >, gridtools::layout_map< 0, 1, 2 >, double >(
            __Size0, __Size1, __Size2)),
        true);
}

TEST(TESTCLASS, copies2Di) {
    EXPECT_EQ((copy_stencils_3D_2D_1D_0D::test< gridtools::layout_map< 0, -1, -1 >,
                  gridtools::layout_map< 0, 1, 2 >,
                  double >(__Size0, __Size1, __Size2)),
        true);
}

TEST(TESTCLASS, copies2Dj) {
    EXPECT_EQ((copy_stencils_3D_2D_1D_0D::test< gridtools::layout_map< -1, 0, -1 >,
                  gridtools::layout_map< 0, 1, 2 >,
                  double >(__Size0, __Size1, __Size2)),
        true);
}

TEST(TESTCLASS, copies2Dk) {
    EXPECT_EQ((copy_stencils_3D_2D_1D_0D::test< gridtools::layout_map< -1, -1, 0 >,
                  gridtools::layout_map< 0, 1, 2 >,
                  double >(__Size0, __Size1, __Size2)),
        true);
}

TEST(TESTCLASS, copies2DScalar) {
    EXPECT_EQ((copy_stencils_3D_2D_1D_0D::test< gridtools::layout_map< -1, -1, -1 >,
                  gridtools::layout_map< 0, 1, 2 >,
                  double >(__Size0, __Size1, __Size2)),
        true);
}

TEST(TESTCLASS, copies3DDst) {
    EXPECT_EQ(
        (copy_stencils_3D_2D_1D_0D::test< gridtools::layout_map< 0, 1, 2 >, gridtools::layout_map< 2, 0, 1 >, double >(
            __Size0, __Size1, __Size2)),
        true);
}

TEST(TESTCLASS, copies3DtransposeDst) {
    EXPECT_EQ(
        (copy_stencils_3D_2D_1D_0D::test< gridtools::layout_map< 2, 1, 0 >, gridtools::layout_map< 2, 0, 1 >, double >(
            __Size0, __Size1, __Size2)),
        true);
}

TEST(TESTCLASS, copies2DijDst) {
    EXPECT_EQ(
        (copy_stencils_3D_2D_1D_0D::test< gridtools::layout_map< 1, 0, -1 >, gridtools::layout_map< 2, 0, 1 >, double >(
            __Size0, __Size1, __Size2)),
        true);
}

TEST(TESTCLASS, copies2DikDst) {
    EXPECT_EQ(
        (copy_stencils_3D_2D_1D_0D::test< gridtools::layout_map< 1, -1, 0 >, gridtools::layout_map< 2, 0, 1 >, double >(
            __Size0, __Size1, __Size2)),
        true);
}

TEST(TESTCLASS, copies2DjkDst) {
    EXPECT_EQ(
        (copy_stencils_3D_2D_1D_0D::test< gridtools::layout_map< -1, 1, 0 >, gridtools::layout_map< 2, 0, 1 >, double >(
            __Size0, __Size1, __Size2)),
        true);
}

TEST(TESTCLASS, copies2DiDst) {
    EXPECT_EQ((copy_stencils_3D_2D_1D_0D::test< gridtools::layout_map< 0, -1, -1 >,
                  gridtools::layout_map< 2, 0, 1 >,
                  double >(__Size0, __Size1, __Size2)),
        true);
}

TEST(TESTCLASS, copies2DjDst) {
    EXPECT_EQ((copy_stencils_3D_2D_1D_0D::test< gridtools::layout_map< -1, 0, -1 >,
                  gridtools::layout_map< 2, 0, 1 >,
                  double >(__Size0, __Size1, __Size2)),
        true);
}

TEST(TESTCLASS, copies2DkDst) {
    EXPECT_EQ((copy_stencils_3D_2D_1D_0D::test< gridtools::layout_map< -1, -1, 0 >,
                  gridtools::layout_map< 2, 0, 1 >,
                  double >(__Size0, __Size1, __Size2)),
        true);
}

TEST(TESTCLASS, copies2DScalarDst) {
    EXPECT_EQ((copy_stencils_3D_2D_1D_0D::test< gridtools::layout_map< -1, -1, -1 >,
                  gridtools::layout_map< 2, 0, 1 >,
                  double >(__Size0, __Size1, __Size2)),
        true);
}

TEST(TESTCLASS, copies3D_bool) {
    EXPECT_EQ(
        (copy_stencils_3D_2D_1D_0D::test< gridtools::layout_map< 0, 1, 2 >, gridtools::layout_map< 0, 1, 2 >, bool >(
            __Size0, __Size1, __Size2)),
        true);
}
