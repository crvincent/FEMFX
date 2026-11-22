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

//---------------------------------------------------------------------------------------
// API for platform atomic operations
//---------------------------------------------------------------------------------------

#pragma once

#include "FEMFXCommon.h"

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace AMD
{
    // Atomic operation interface

    static FM_FORCE_INLINE uint FmAtomicIncrement(uint* pValue)
    {
#if defined(_MSC_VER)
        return (uint)_InterlockedIncrement(reinterpret_cast<volatile long*>(pValue));
#else
        return (uint)__atomic_add_fetch(pValue, 1u, __ATOMIC_SEQ_CST);
#endif
    }
    static FM_FORCE_INLINE uint FmAtomicDecrement(uint* pValue)
    {
#if defined(_MSC_VER)
        return (uint)_InterlockedDecrement(reinterpret_cast<volatile long*>(pValue));
#else
        return (uint)__atomic_sub_fetch(pValue, 1u, __ATOMIC_SEQ_CST);
#endif
    }
    static FM_FORCE_INLINE uint FmAtomicAdd(uint* pValue, uint addedValue)
    {
#if defined(_MSC_VER)
        return (uint)_InterlockedExchangeAdd(
            reinterpret_cast<volatile long*>(pValue), static_cast<long>(addedValue)) + addedValue;
#else
        return (uint)__atomic_add_fetch(pValue, addedValue, __ATOMIC_SEQ_CST);
#endif
    }
    static FM_FORCE_INLINE uint FmAtomicSub(uint* pValue, uint subtractedValue)
    {
#if defined(_MSC_VER)
        return (uint)_InterlockedExchangeAdd(
            reinterpret_cast<volatile long*>(pValue), -static_cast<long>(subtractedValue)) - subtractedValue;
#else
        return (uint)__atomic_sub_fetch(pValue, subtractedValue, __ATOMIC_SEQ_CST);
#endif
    }
    static FM_FORCE_INLINE uint FmAtomicOr(uint* pValue, uint orValue)
    {
#if defined(_MSC_VER)
        return (uint)_InterlockedOr(
            reinterpret_cast<volatile long*>(pValue), static_cast<long>(orValue));
#else
        return (uint)__atomic_fetch_or(pValue, orValue, __ATOMIC_SEQ_CST);
#endif
    }
    static FM_FORCE_INLINE uint FmAtomicCompareExchange(uint* pValue, uint newValue, uint compareValue)
    {
#if defined(_MSC_VER)
        return (uint)_InterlockedCompareExchange(
            reinterpret_cast<volatile long*>(pValue),
            static_cast<long>(newValue),
            static_cast<long>(compareValue));
#else
        __atomic_compare_exchange_n(
            pValue, &compareValue, newValue, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        return (uint)compareValue;
#endif
    }
    static FM_FORCE_INLINE uint FmAtomicRead(uint* pValue)
    {
#if defined(_MSC_VER)
        return (uint)_InterlockedCompareExchange(
            reinterpret_cast<volatile long*>(pValue), 0, 0);
#else
        return (uint)__atomic_load_n(pValue, __ATOMIC_SEQ_CST);
#endif
    }
    static FM_FORCE_INLINE uint FmAtomicWrite(uint* pValue, uint newValue)
    {
#if defined(_MSC_VER)
        return (uint)_InterlockedExchange(
            reinterpret_cast<volatile long*>(pValue), static_cast<long>(newValue));
#else
        return (uint)__atomic_exchange_n(pValue, newValue, __ATOMIC_SEQ_CST);
#endif
    }

    // Atomically compute max
    void FmAtomicMax(uint* pValue, uint newValue);
}