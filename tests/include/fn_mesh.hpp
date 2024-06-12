/*
 * GridTools
 *
 * Copyright (c) 2014-2023, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <cassert>

#include <gridtools/storage/builder.hpp>
#include <type_traits>

namespace gridtools {

    struct v2e {};
    struct e2v {};

    namespace storage {

        template <class StorageTraits>
        struct nb_table_storage_traits {
            friend decltype(storage_is_host_referenceable(std::declval<StorageTraits>())) storage_is_host_referenceable(
                nb_table_storage_traits<StorageTraits>) {
                return {};
            }

            friend layout_map<1, 0> storage_layout(
                nb_table_storage_traits<StorageTraits>, std::integral_constant<size_t, 2>) {
                return {};
            }

            friend decltype(storage_alignment(std::declval<StorageTraits>())) storage_alignment(
                nb_table_storage_traits<StorageTraits>) {
                return {};
            }

            template <class LazyType, class T = typename LazyType::type>
            friend auto storage_allocate(nb_table_storage_traits<StorageTraits>, LazyType type, size_t size) {
                return storage_allocate(StorageTraits{}, type, size);
            }

            template <class T>
            friend void storage_update_target(
                nb_table_storage_traits<StorageTraits>, T *dst, T const *src, size_t size) {
                storage_update_target(StorageTraits{}, dst, src, size);
            }

            template <class T>
            friend void storage_update_host(nb_table_storage_traits<StorageTraits>, T *dst, T const *src, size_t size) {
                storage_update_host(StorageTraits{}, dst, src, size);
            }

            template <class T, class Info>
            friend auto storage_make_target_view(nb_table_storage_traits<StorageTraits>, T *ptr, Info const &info) {
                return storage_make_target_view(StorageTraits{}, ptr, info);
            }
        };

    } // namespace storage

    template <class StorageTraits, class FloatType>
    class structured_unstructured_mesh {
        int m_nx, m_ny, m_nz;

        constexpr auto v2e_initializer() const {
            return [nx = m_nx, ny = m_ny](int const vertex, int const neighbor) {
                assert(vertex >= 0 && vertex < nx * ny);
                int const nxedges = (nx - 1) * ny;
                int const nyedges = nx * (ny - 1);
                int const i = vertex % nx;
                int const j = vertex / nx;
                int n = 0;
                if (i > 0 && neighbor == n++)
                    return (i - 1) + (nx - 1) * j;
                if (i < nx - 1 && neighbor == n++)
                    return i + (nx - 1) * j;
                if (j > 0 && neighbor == n++)
                    return nxedges + i + nx * (j - 1);
                if (j < ny - 1 && neighbor == n++)
                    return nxedges + i + nx * j;
                if (i < nx - 1 && j > 0 && neighbor == n++)
                    return nxedges + nyedges + i + (nx - 1) * (j - 1);
                if (i > 0 && j < ny - 1 && neighbor == n++)
                    return nxedges + nyedges + (i - 1) + (nx - 1) * j;
                return -1;
            };
        }

        constexpr auto e2v_initializer() const {
            return [nx = m_nx, ny = m_ny](int edge, int const neighbor) {
                int const nxedges = (nx - 1) * ny;
                int const nyedges = nx * (ny - 1);
                [[maybe_unused]] int const nxyedges = (nx - 1) * (ny - 1);
                assert(edge >= 0 && edge < nxedges + nyedges + nxyedges);
                if (edge < nxedges) {
                    int i = edge % (nx - 1);
                    int j = edge / (nx - 1);
                    return neighbor == 0 ? i + nx * j : i + 1 + nx * j;
                }
                edge -= nxedges;
                if (edge < nyedges) {
                    int i = edge % nx;
                    int j = edge / nx;
                    return neighbor == 0 ? i + nx * j : i + nx * (j + 1);
                }
                edge -= nyedges;
                assert(edge < nxyedges);
                int i = edge % (nx - 1);
                int j = edge / (nx - 1);
                return neighbor == 0 ? i + 1 + nx * j : i + nx * (j + 1);
            };
        }

      public:
        using max_v2e_neighbors_t = std::integral_constant<int, 6>;
        using max_e2v_neighbors_t = std::integral_constant<int, 2>;

        constexpr structured_unstructured_mesh(int nx, int ny, int nz) : m_nx(nx), m_ny(ny), m_nz(nz) {}

        constexpr int nvertices() const { return m_nx * m_ny; }
        constexpr int nedges() const {
            int nxedges = (m_nx - 1) * m_ny;
            int nyedges = m_nx * (m_ny - 1);
            int nxyedges = (m_nx - 1) * (m_ny - 1);
            return nxedges + nyedges + nxyedges;
        }
        constexpr int nlevels() const { return m_nz; }

        template <class T = FloatType,
            class Init,
            class... Dims,
            std::enable_if_t<!(std::is_integral_v<Init> || is_integral_constant<Init>::value), int> = 0>
        auto make_storage(Init const &init, Dims... dims) const {
            return storage::builder<StorageTraits>.dimensions(dims...).template type<T>().initializer(init).unknown_id().build();
        }

        template <class T = FloatType,
            class... Dims,
            std::enable_if_t<std::conjunction_v<std::bool_constant<std::is_integral<Dims>::value ||
                                                                   is_integral_constant<Dims>::value>...>,
                int> = 0>
        auto make_storage(Dims... dims) const {
            return make_storage<T>([](int...) { return T(); }, dims...);
        }

        template <class T = FloatType, class... Args>
        auto make_const_storage(Args &&...args) const {
            return make_storage<T const>(std::forward<Args>(args)...);
        }

        auto v2e_table() const {
            return storage::builder<storage::nb_table_storage_traits<StorageTraits>>.dimensions(nvertices(), max_v2e_neighbors_t()).template type<int>().initializer(v2e_initializer()).unknown_id().build();
        }

        auto e2v_table() const {
            return storage::builder<storage::nb_table_storage_traits<StorageTraits>>.dimensions(nedges(), max_e2v_neighbors_t()).template type<int>().initializer(e2v_initializer()).unknown_id().build();
        }
    };

} // namespace gridtools
