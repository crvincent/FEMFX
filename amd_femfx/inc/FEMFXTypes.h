/*
MIT License

Copyright (c) 2019 Advanced Micro Devices, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <algorithm>
#include <assert.h>

#if defined(_WIN32) || defined(_WIN64)
#define FM_PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
#define FM_PLATFORM_MACOS 1
#elif defined(__linux__) || defined(__unix__)
#define FM_PLATFORM_LINUX 1
#else
#define FM_PLATFORM_UNKNOWN 1
#endif

#ifndef FM_FORCE_INLINE
#if defined(_MSC_VER)
#define FM_FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define FM_FORCE_INLINE inline __attribute__((always_inline))
#else
#define FM_FORCE_INLINE inline
#endif
#endif

#ifndef FM_RESTRICT
#if defined(_MSC_VER)
#define FM_RESTRICT __restrict
#elif defined(__GNUC__) || defined(__clang__)
#define FM_RESTRICT __restrict__
#else
#define FM_RESTRICT
#endif
#endif

#ifndef FM_ALIGN_OF
#if defined(_MSC_VER)
#define FM_ALIGN_OF(x) __alignof(x)
#else
#define FM_ALIGN_OF(x) alignof(x)
#endif
#endif

#ifndef FM_ALIGN
#if defined(_MSC_VER)
#define FM_ALIGN(x) __declspec(align(x))
#else
#define FM_ALIGN(x) alignas(x)
#endif
#endif

#include "FEMFXVectorMath.h"

#define FM_ALIGN_END(x)
#if defined(_MSC_VER)
#define FM_THREAD_LOCAL_STORAGE __declspec(thread)
#else
#define FM_THREAD_LOCAL_STORAGE thread_local
#endif

#ifdef _DEBUG
#define FM_ASSERT(x) assert(x)
#else
#define FM_ASSERT(x) ((void)0)
#endif
#define FM_STATIC_ASSERT(x) static_assert(x, #x)

#define FM_DELETE(ptr) if (ptr) { delete (ptr); (ptr) = NULL; }
#define FM_DELETE_ARRAY(ptr) if (ptr) { delete [] (ptr); (ptr) = NULL; }

#define FM_PI 3.14159265358979323846f

namespace AMD
{
    typedef uint32_t uint;

    // Integer types in struct matching size/alignment of cache block
    struct FmAtomicUint
    {
        FM_ALIGN(64) uint32_t val FM_ALIGN_END(64);
        FmAtomicUint() {}
        FmAtomicUint(uint32_t inVal) { val = inVal; }
    };

    struct FmAtomicUint64
    {
        FM_ALIGN(64) uint64_t val FM_ALIGN_END(64);
        FmAtomicUint64() {}
        FmAtomicUint64(uint64_t inVal) { val = inVal; }
    };

    // Min/max
    static FM_FORCE_INLINE float FmMinFloat(float a, float b) { return std::min(a, b); }
    static FM_FORCE_INLINE float FmMaxFloat(float a, float b) { return std::max(a, b); }
    static FM_FORCE_INLINE int32_t FmMinInt(int32_t a, int32_t b) { return std::min(a, b); }
    static FM_FORCE_INLINE int32_t FmMaxInt(int32_t a, int32_t b) { return std::max(a, b); }
    static FM_FORCE_INLINE uint32_t FmMinUint(uint32_t a, uint32_t b) { return std::min(a, b); }
    static FM_FORCE_INLINE uint32_t FmMaxUint(uint32_t a, uint32_t b) { return std::max(a, b); }
}
