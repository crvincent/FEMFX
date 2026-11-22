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

#include <stdint.h>

#define SIMD_UTILS_USE_SSE3 0
#define SIMD_UTILS_USE_SSE4 0
#define SIMD_UTILS_USE_FMA 0
#define SIMD_UTILS_USE_AVX 0

#define SIMD_UTILS_ALIGN16
#if defined(_MSC_VER)
#define SIMD_UTILS_FORCE_INLINE __forceinline
#else
#define SIMD_UTILS_FORCE_INLINE inline __attribute__((always_inline))
#endif

struct FEMFXScalar128
{
    float f[4];
    FEMFXScalar128() {}
    FEMFXScalar128(float x0, float x1, float x2, float x3)
    {
        f[0] = x0;
        f[1] = x1;
        f[2] = x2;
        f[3] = x3;
    }
};

struct FEMFXScalar128i
{
    int32_t i[4];
    FEMFXScalar128i() {}
    FEMFXScalar128i(int32_t x0, int32_t x1, int32_t x2, int32_t x3)
    {
        i[0] = x0;
        i[1] = x1;
        i[2] = x2;
        i[3] = x3;
    }
};

typedef FEMFXScalar128 FmSimdFloat128;
typedef FEMFXScalar128i FmSimdInt128;

union Simd128Union
{
    FmSimdInt128 vi;
    FmSimdFloat128 vf;
    int32_t i[4];
    float f[4];
    Simd128Union() {}
};

static SIMD_UTILS_FORCE_INLINE float simd_getx_ps(FmSimdFloat128 value)
{ return value.f[0]; }
static SIMD_UTILS_FORCE_INLINE float simd_gety_ps(FmSimdFloat128 value)
{ return value.f[1]; }
static SIMD_UTILS_FORCE_INLINE float simd_getz_ps(FmSimdFloat128 value)
{ return value.f[2]; }
static SIMD_UTILS_FORCE_INLINE float simd_getw_ps(FmSimdFloat128 value)
{ return value.f[3]; }
static SIMD_UTILS_FORCE_INLINE float simd_getelem_ps(
    FmSimdFloat128 value, int index)
{ return value.f[index & 3]; }
static SIMD_UTILS_FORCE_INLINE int32_t simd_getelem_epi32(
    FmSimdInt128 value, int index)
{ return value.i[index & 3]; }

static SIMD_UTILS_FORCE_INLINE FmSimdFloat128 simd_shuffle_xyxy(
    FmSimdFloat128 a, FmSimdFloat128 b)
{ return FmSimdFloat128(a.f[0], a.f[1], b.f[0], b.f[1]); }
static SIMD_UTILS_FORCE_INLINE FmSimdFloat128 simd_shuffle_yzyz(
    FmSimdFloat128 a, FmSimdFloat128 b)
{ return FmSimdFloat128(a.f[1], a.f[2], b.f[1], b.f[2]); }
static SIMD_UTILS_FORCE_INLINE FmSimdFloat128 simd_shuffle_zxzx(
    FmSimdFloat128 a, FmSimdFloat128 b)
{ return FmSimdFloat128(a.f[2], a.f[0], b.f[2], b.f[0]); }

static SIMD_UTILS_FORCE_INLINE void simd_set_soa_vec3(
    FmSimdFloat128* x, FmSimdFloat128* y, FmSimdFloat128* z,
    const float* vectors)
{
    *x = FmSimdFloat128(vectors[0], vectors[3], vectors[6], vectors[9]);
    *y = FmSimdFloat128(vectors[1], vectors[4], vectors[7], vectors[10]);
    *z = FmSimdFloat128(vectors[2], vectors[5], vectors[8], vectors[11]);
}

static SIMD_UTILS_FORCE_INLINE void simd_set_soa_vec4(
    FmSimdFloat128* x, FmSimdFloat128* y, FmSimdFloat128* z,
    FmSimdFloat128* w, const float* vectors)
{
    *x = FmSimdFloat128(vectors[0], vectors[4], vectors[8], vectors[12]);
    *y = FmSimdFloat128(vectors[1], vectors[5], vectors[9], vectors[13]);
    *z = FmSimdFloat128(vectors[2], vectors[6], vectors[10], vectors[14]);
    *w = FmSimdFloat128(vectors[3], vectors[7], vectors[11], vectors[15]);
}
