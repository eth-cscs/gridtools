/*
 * GridTools
 *
 * Copyright (c) 2014-2021, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <gridtools/fn/unstructured2.hpp>

#include <gtest/gtest.h>

#include <gridtools/fn/backend2/gpu.hpp>
#include <gridtools/sid/synthetic.hpp>

namespace gridtools::fn {
    namespace {
        using namespace literals;
        using sid::property;

        template <int I>
        using int_t = integral_constant<int, I>;

        template <class C>
        struct stencil {
            constexpr auto operator()() const {
                return [](auto const &in) { return reduce(C(), std::plus(), 0, in); };
            }
        };

        struct vertex {};
        struct edge {};
        struct v2v {
            static constexpr int max_neighbors = 3;
        };
        struct v2e {
            static constexpr int max_neighbors = 2;
        };

        using block_sizes_t = meta::list<meta::list<vertex, int_t<32>>, meta::list<unstructured::dim::k, int_t<1>>>;

        TEST(unstructured, v2v_sum) {
            auto apply_stencil = [](auto executor, auto &out, auto const &in) {
                executor().arg(out).arg(in).assign(0_c, stencil<v2v>(), 1_c);
            };
            auto fencil = [&](auto const &v2v_table, int nvertices, int nlevels, auto &out, auto const &in) {
                auto v2v_conn = connectivity<v2v, vertex, vertex>(v2v_table);
                auto domain = unstructured_domain<vertex>(nvertices, nlevels, v2v_conn);
                auto backend = make_backend(backend::gpu<block_sizes_t>(), domain);
                apply_stencil(backend.stencil_executor(), out, in);
            };

            auto v2v_table = cuda_util::cuda_malloc<int>(3 * 3);
            int v2v_tableh[3][3] = {{1, 2, -1}, {0, 2, -1}, {0, 1, -1}};
            cudaMemcpy(v2v_table.get(), v2v_tableh, 3 * 3 * sizeof(int), cudaMemcpyHostToDevice);
            auto v2v_conn =
                sid::synthetic()
                    .set<property::origin>(sid::host_device::make_simple_ptr_holder(v2v_table.get()))
                    .set<property::strides>(hymap::keys<vertex, unstructured::dim::neighbor>::make_values(3_c, 1_c));

            auto in = cuda_util::cuda_malloc<int>(3 * 5);
            auto out = cuda_util::cuda_malloc<int>(3 * 5);
            int inh[3][5], outh[3][5] = {};
            for (int v = 0; v < 3; ++v)
                for (int k = 0; k < 5; ++k)
                    inh[v][k] = 5 * v + k;
            cudaMemcpy(in.get(), inh, 3 * 5 * sizeof(int), cudaMemcpyHostToDevice);

            auto as_synthetic = [](int *x) {
                return sid::synthetic()
                    .set<property::origin>(sid::host_device::make_simple_ptr_holder(x))
                    .set<property::strides>(hymap::keys<vertex, unstructured::dim::k>::make_values(5_c, 1_c));
            };
            auto in_s = as_synthetic(in.get());
            auto out_s = as_synthetic(out.get());

            fencil(v2v_conn, 3, 5, out_s, in_s);
            cudaMemcpy(outh, out.get(), 3 * 5 * sizeof(int), cudaMemcpyDeviceToHost);

            for (int v = 0; v < 3; ++v)
                for (int k = 0; k < 5; ++k) {
                    int nbsum = 0;
                    for (int i = 0; i < 3; ++i) {
                        int nb = v2v_tableh[v][i];
                        if (nb != -1)
                            nbsum += inh[nb][k];
                    }
                    EXPECT_EQ(outh[v][k], nbsum);
                }
        }

        TEST(unstructured, v2e_sum) {
            auto apply_stencil = [](auto executor, auto &out, auto const &in) {
                executor().arg(out).arg(in).assign(0_c, stencil<v2e>(), 1_c);
            };
            auto fencil = [&](auto const &v2e_table, int nvertices, int nlevels, auto &out, auto const &in) {
                auto v2e_conn = connectivity<v2e, vertex, edge>(v2e_table);
                auto domain = unstructured_domain<vertex>(nvertices, nlevels, v2e_conn);
                auto backend = make_backend(backend::gpu<block_sizes_t>(), domain);
                apply_stencil(backend.stencil_executor(), out, in);
            };

            auto v2e_table = cuda_util::cuda_malloc<int>(3 * 2);
            int v2e_tableh[3][2] = {{0, 2}, {0, 1}, {1, 2}};
            cudaMemcpy(v2e_table.get(), v2e_tableh, 3 * 2 * sizeof(int), cudaMemcpyHostToDevice);
            auto v2e_conn =
                sid::synthetic()
                    .set<property::origin>(sid::host_device::make_simple_ptr_holder(v2e_table.get()))
                    .set<property::strides>(hymap::keys<vertex, unstructured::dim::neighbor>::make_values(2_c, 1_c));

            auto in = cuda_util::cuda_malloc<int>(3 * 5);
            auto out = cuda_util::cuda_malloc<int>(3 * 5);
            int inh[3][5], outh[3][5] = {};
            for (int e = 0; e < 3; ++e)
                for (int k = 0; k < 5; ++k)
                    inh[e][k] = 5 * e + k;
            cudaMemcpy(in.get(), inh, 3 * 5 * sizeof(int), cudaMemcpyHostToDevice);

            auto as_synthetic = [](int *x, auto loc) {
                return sid::synthetic()
                    .set<property::origin>(sid::host_device::make_simple_ptr_holder(x))
                    .set<property::strides>(hymap::keys<decltype(loc), unstructured::dim::k>::make_values(5_c, 1_c));
            };
            auto in_s = as_synthetic(in.get(), edge());
            auto out_s = as_synthetic(out.get(), vertex());

            GT_CUDA_CHECK(cudaDeviceSynchronize());
            fencil(v2e_conn, 3, 5, out_s, in_s);
            GT_CUDA_CHECK(cudaDeviceSynchronize());
            cudaMemcpy(outh, out.get(), 3 * 5 * sizeof(int), cudaMemcpyDeviceToHost);

            for (int v = 0; v < 3; ++v)
                for (int k = 0; k < 5; ++k) {
                    int nbsum = 0;
                    for (int i = 0; i < 2; ++i) {
                        int nb = v2e_tableh[v][i];
                        nbsum += inh[nb][k];
                    }
                    EXPECT_EQ(outh[v][k], nbsum);
                }
        }

    } // namespace
} // namespace gridtools::fn
