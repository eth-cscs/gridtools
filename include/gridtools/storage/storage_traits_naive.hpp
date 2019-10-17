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

#include "../common/gt_assert.hpp"
#include "../common/selector.hpp"
#include "./common/definitions.hpp"
#include "./common/storage_traits_metafunctions.hpp"
#include "./storage_host/host_storage.hpp"

namespace gridtools {
    /** \ingroup storage
     * @{
     */

    template <class Backend>
    struct storage_traits_from_id;

    /** @brief storage traits for the Host backend*/
    template <>
    struct storage_traits_from_id<backend::naive> {
        static constexpr uint_t default_alignment = 1;

        template <typename ValueType>
        using select_storage = host_storage<ValueType>;

        template <uint_t Dims>
        using select_layout = typename get_layout<Dims, true>::type;
    };

    /**
     * @}
     */
} // namespace gridtools
