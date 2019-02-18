/*
 * GridTools Libraries
 * Copyright (c) 2019, ETH Zurich
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// DON'T USE #pragma once HERE!!!

#if !defined(GT_FILENAME)
#error GT_FILENAME is not defined
#endif

#if defined(GT_TARGET_ITERATING)
#error nesting target iterating is not supported
#endif

#if defined(GT_TARGET)
#error GT_TARGET should not be defined outside of this file
#endif

#if defined(GT_TARGET_NAMESPACE)
#error GT_TARGET_NAMESPACE should not be defined outside of this file
#endif

#define GT_TARGET_ITERATING

#ifdef __CUDACC__

#define GT_TARGET_NAMESPACE_NAME host
#define GT_TARGET_NAMESPACE inline namespace host
#define GT_TARGET GT_HOST
#include GT_FILENAME
#undef GT_TARGET
#undef GT_TARGET_NAMESPACE
#undef GT_TARGET_NAMESPACE_NAME

#define GT_TARGET_NAMESPACE_NAME host_device
#define GT_TARGET_NAMESPACE namespace host_device
#define GT_TARGET GT_HOST_DEVICE
#include GT_FILENAME
#undef GT_TARGET
#undef GT_TARGET_NAMESPACE
#undef GT_TARGET_NAMESPACE_NAME

#define GT_TARGET_NAMESPACE_NAME device
#define GT_TARGET_NAMESPACE namespace device
#define GT_TARGET GT_DEVICE
#include GT_FILENAME
#undef GT_TARGET
#undef GT_TARGET_NAMESPACE
#undef GT_TARGET_NAMESPACE_NAME

#else

#define GT_TARGET_NAMESPACE_NAME host
#define GT_TARGET_NAMESPACE   \
    inline namespace host {}  \
    namespace device {        \
        using namespace host; \
    }                         \
    namespace host_device {   \
        using namespace host; \
    }                         \
    inline namespace host
#define GT_TARGET GT_HOST
#include GT_FILENAME
#undef GT_TARGET
#undef GT_TARGET_NAMESPACE
#undef GT_TARGET_NAMESPACE_NAME

#endif

#undef GT_TARGET_ITERATING
