#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "FEMFXHighway.cpp"

#include <cmath>

#include "hwy/foreach_target.h"
#include "hwy/highway.h"
#include "hwy/contrib/math/math-inl.h"

#include "FEMFXHighway.h"
#include "soa_highway.h"

HWY_BEFORE_NAMESPACE();
namespace AMD {
namespace HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

template <class D>
struct HMat3
{
    hn::Vec<D> e[9];
};

template <class D>
static HWY_INLINE hn::Vec<D> LoadBatch(const D d, const float* values, size_t lane, size_t count)
{
    return hn::LoadN(d, values + lane, count - lane);
}

template <class D>
static HWY_INLINE void StoreBatch(const D d, hn::Vec<D> value, float* values, size_t lane, size_t count)
{
    hn::StoreN(value, d, values + lane, count - lane);
}

template <class D>
static HWY_INLINE HMat3<D> IdentityMat3(const D d)
{
    HMat3<D> result = {};
    result.e[0] = result.e[4] = result.e[8] = hn::Set(d, 1.0f);
    result.e[1] = result.e[2] = result.e[3] = result.e[5] =
        result.e[6] = result.e[7] = hn::Zero(d);
    return result;
}

template <class D>
static HWY_INLINE HMat3<D> LoadMat3(const D d, const float* values, size_t lane, size_t count)
{
    HMat3<D> result;
    for (size_t i = 0; i < 9; ++i)
        result.e[i] = LoadBatch(d, values + i * FmHighwayMaxLanes, lane, count);
    return result;
}

template <class D>
static HWY_INLINE void StoreMat3(const D d, const HMat3<D>& matrix, float* values, size_t lane, size_t count)
{
    for (size_t i = 0; i < 9; ++i)
        StoreBatch(d, matrix.e[i], values + i * FmHighwayMaxLanes, lane, count);
}

template <class D>
static HWY_INLINE HMat3<D> MulMat3(const D d, const HMat3<D>& a, const HMat3<D>& b)
{
    HMat3<D> result;
    for (int col = 0; col < 3; ++col)
    {
        for (int row = 0; row < 3; ++row)
        {
            result.e[col * 3 + row] =
                hn::MulAdd(a.e[row], b.e[col * 3],
                    hn::MulAdd(a.e[3 + row], b.e[col * 3 + 1],
                        a.e[6 + row] * b.e[col * 3 + 2]));
        }
    }
    return result;
}

template <class D>
static HWY_INLINE HMat3<D> TransposeMat3(const HMat3<D>& matrix)
{
    HMat3<D> result = matrix;
    result.e[1] = matrix.e[3];
    result.e[2] = matrix.e[6];
    result.e[3] = matrix.e[1];
    result.e[5] = matrix.e[7];
    result.e[6] = matrix.e[2];
    result.e[7] = matrix.e[5];
    return result;
}

template <class D, class V>
static HWY_INLINE void ApproxGivens(const D d, V* sine, V* cosine, V a00, V a01, V a11)
{
    const V delta = a00 - a11;
    const V delta2 = delta * delta;
    const V off2 = a01 * a01;
    const auto ordinary = hn::Lt(off2, delta2);
    const V omega = hn::Set(d, 1.0f) / hn::Sqrt(off2 + delta2);
    *sine = hn::IfThenElse(ordinary, omega * a01,
        hn::Set(d, 0.7071067811865475244f));
    *cosine = hn::IfThenElse(ordinary, omega * delta,
        hn::Set(d, 0.7071067811865475244f));
}

template <class D>
static HWY_INLINE HMat3<D> GivensMat(const D d, int p, hn::Vec<D> sine, hn::Vec<D> cosine)
{
    HMat3<D> result = IdentityMat3(d);
    if (p == 0)
    {
        result.e[0] = cosine;
        result.e[1] = sine;
        result.e[3] = hn::Neg(sine);
        result.e[4] = cosine;
    }
    else
    {
        result.e[4] = cosine;
        result.e[5] = sine;
        result.e[7] = hn::Neg(sine);
        result.e[8] = cosine;
    }
    return result;
}

template <class D>
static HWY_INLINE void EigenSymmImpl(const D d, const HMat3<D>& matrix,
    hn::Vec<D> eigenvalues[3], HMat3<D>* eigenvectors)
{
    hn::Vec<D> s00 = matrix.e[0], s01 = matrix.e[3], s02 = matrix.e[6];
    hn::Vec<D> s11 = matrix.e[4], s12 = matrix.e[7], s22 = matrix.e[8];
    HMat3<D> v = IdentityMat3(d);

    for (int iteration = 0; iteration < 4; ++iteration)
    {
        hn::Vec<D> sine, cosine;
        ApproxGivens(d, &sine, &cosine, s00, s01, s11);
        v = MulMat3(d, v, GivensMat(d, 0, sine, cosine));
        const hn::Vec<D> n00 = sine * sine * s11 + sine * cosine * s01 + cosine * sine * s01 + cosine * cosine * s00;
        const hn::Vec<D> n01 = sine * cosine * s11 - sine * sine * s01 + cosine * cosine * s01 - cosine * sine * s00;
        const hn::Vec<D> n02 = sine * s12 + cosine * s02;
        const hn::Vec<D> n11 = cosine * cosine * s11 - cosine * sine * s01 - sine * cosine * s01 + sine * sine * s00;
        const hn::Vec<D> n12 = cosine * s12 - sine * s02;
        s00 = n00; s01 = n01; s02 = n02; s11 = n11; s12 = n12;

        ApproxGivens(d, &sine, &cosine, s00, s02, s22);
        HMat3<D> g = IdentityMat3(d);
        g.e[0] = cosine; g.e[2] = sine; g.e[6] = hn::Neg(sine); g.e[8] = cosine;
        v = MulMat3(d, v, g);
        const hn::Vec<D> q00 = sine * sine * s22 + sine * cosine * s02 + cosine * sine * s02 + cosine * cosine * s00;
        const hn::Vec<D> q01 = sine * s12 + cosine * s01;
        const hn::Vec<D> q02 = sine * cosine * s22 - sine * sine * s02 + cosine * cosine * s02 - cosine * sine * s00;
        const hn::Vec<D> q12 = cosine * s12 - sine * s01;
        const hn::Vec<D> q22 = cosine * cosine * s22 - cosine * sine * s02 - sine * cosine * s02 + sine * sine * s00;
        s00 = q00; s01 = q01; s02 = q02; s12 = q12; s22 = q22;

        ApproxGivens(d, &sine, &cosine, s11, s12, s22);
        g = GivensMat(d, 1, sine, cosine);
        v = MulMat3(d, v, g);
        const hn::Vec<D> r01 = sine * s02 + cosine * s01;
        const hn::Vec<D> r02 = cosine * s02 - sine * s01;
        const hn::Vec<D> r11 = sine * sine * s22 + sine * cosine * s12 + cosine * sine * s12 + cosine * cosine * s11;
        const hn::Vec<D> r12 = sine * cosine * s22 - sine * sine * s12 + cosine * cosine * s12 - cosine * sine * s11;
        const hn::Vec<D> r22 = cosine * cosine * s22 - cosine * sine * s12 - sine * cosine * s12 + sine * sine * s11;
        s01 = r01; s02 = r02; s11 = r11; s12 = r12; s22 = r22;
    }

    hn::Vec<D> eval[3] = {s00, s11, s22};

    for (int i = 0; i < 2; ++i)
    {
        for (int j = i + 1; j < 3; ++j)
        {
            const auto swap = hn::Lt(hn::Abs(eval[i]), hn::Abs(eval[j]));
            for (int component = 0; component < 3; ++component)
            {
                const hn::Vec<D> oldVec = v.e[3 * i + component];
                v.e[3 * i + component] =
                    hn::IfThenElse(swap, v.e[3 * j + component], oldVec);
                v.e[3 * j + component] =
                    hn::IfThenElse(swap, hn::Neg(oldVec), v.e[3 * j + component]);
            }
            const hn::Vec<D> oldEval = eval[i];
            eval[i] = hn::IfThenElse(swap, eval[j], eval[i]);
            eval[j] = hn::IfThenElse(swap, hn::Neg(oldEval), eval[j]);
        }
    }
    for (int i = 0; i < 3; ++i)
        eigenvalues[i] = eval[i];
    *eigenvectors = v;
}

void FmHighwayTransposeAosToSoaImpl(size_t components, const float* aos, float* soa, size_t count)
{
    hn::CappedTag<float, 8> d;
    const size_t lanes = hn::Lanes(d);
    for (size_t lane = 0; lane < count; lane += lanes)
    {
        if (components == 3)
        {
            hn::Vec<decltype(d)> x, y, z;
            hn::LoadInterleaved3(d, aos + 3 * lane, x, y, z);
            hn::StoreU(x, d, soa + lane);
            hn::StoreU(y, d, soa + FmHighwayMaxLanes + lane);
            hn::StoreU(z, d, soa + 2 * FmHighwayMaxLanes + lane);
        }
        else
        {
            hn::Vec<decltype(d)> x, y, z, w;
            hn::LoadInterleaved4(d, aos + 4 * lane, x, y, z, w);
            hn::StoreU(x, d, soa + lane);
            hn::StoreU(y, d, soa + FmHighwayMaxLanes + lane);
            hn::StoreU(z, d, soa + 2 * FmHighwayMaxLanes + lane);
            hn::StoreU(w, d, soa + 3 * FmHighwayMaxLanes + lane);
        }
    }
}

void FmHighwayEigenSymm3x3Impl(const float* matrix, float* eigenvalues, float* eigenvectors, size_t count)
{
    hn::CappedTag<float, 8> d;
    for (size_t lane = 0; lane < count; lane += hn::Lanes(d))
    {
        HMat3<decltype(d)> vectors;
        hn::Vec<decltype(d)> values[3];
        EigenSymmImpl(d, LoadMat3(d, matrix, lane, count), values, &vectors);
        for (int i = 0; i < 3; ++i)
            StoreBatch(d, values[i], eigenvalues + i * FmHighwayMaxLanes, lane, count);
        StoreMat3(d, vectors, eigenvectors, lane, count);
    }
}

template <class D>
static HWY_INLINE void QRGivens(const D d, hn::Vec<D>* sine, hn::Vec<D>* cosine,
    hn::Vec<D> apq, hn::Vec<D> aqq, hn::Vec<D> tolerance)
{
    const hn::Vec<D> sum = apq * apq + aqq * aqq;
    const hn::Vec<D> inverse = hn::Set(d, 1.0f) / hn::Sqrt(sum);
    const auto tiny = hn::Lt(sum, tolerance);
    *sine = hn::IfThenElse(tiny, hn::Zero(d), apq * inverse);
    *cosine = hn::IfThenElse(tiny, hn::Set(d, 1.0f), aqq * inverse);
}

void FmHighwaySvd3x3Impl(const float* matrix, float* u, float* singularValues, float* v,
    const float* tolerance, size_t count)
{
    hn::CappedTag<float, 8> d;
    for (size_t lane = 0; lane < count; lane += hn::Lanes(d))
    {
        const HMat3<decltype(d)> mat = LoadMat3(d, matrix, lane, count);
        const HMat3<decltype(d)> transposeMat = TransposeMat3(mat);
        HMat3<decltype(d)> vectors;
        hn::Vec<decltype(d)> eigenvalues[3];
        EigenSymmImpl(d, MulMat3(d, transposeMat, mat), eigenvalues, &vectors);
        HMat3<decltype(d)> b = MulMat3(d, mat, vectors);
        HMat3<decltype(d)> q = IdentityMat3(d);
        const hn::Vec<decltype(d)> tol = LoadN(d, tolerance + lane, count - lane);
        hn::Vec<decltype(d)> sine, cosine;

        QRGivens(d, &sine, &cosine, b.e[1], b.e[0], tol);
        HMat3<decltype(d)> givens = GivensMat(d, 0, sine, cosine);
        q = MulMat3(d, q, givens);
        b = MulMat3(d, TransposeMat3(givens), b);

        QRGivens(d, &sine, &cosine, b.e[2], b.e[0], tol);
        givens = IdentityMat3(d);
        givens.e[0] = cosine; givens.e[2] = sine; givens.e[6] = hn::Neg(sine); givens.e[8] = cosine;
        q = MulMat3(d, q, givens);
        b = MulMat3(d, TransposeMat3(givens), b);

        QRGivens(d, &sine, &cosine, b.e[5], b.e[4], tol);
        givens = GivensMat(d, 1, sine, cosine);
        q = MulMat3(d, q, givens);
        b = MulMat3(d, TransposeMat3(givens), b);

        StoreMat3(d, q, u, lane, count);
        StoreMat3(d, vectors, v, lane, count);
        StoreBatch(d, b.e[0], singularValues, lane, count);
        StoreBatch(d, b.e[4], singularValues + FmHighwayMaxLanes, lane, count);
        StoreBatch(d, b.e[8], singularValues + 2 * FmHighwayMaxLanes, lane, count);
    }
}

void FmHighwayComputeShapeParamsImpl(
    const float* positions, float* barycentricMatrix, float* determinant, size_t count)
{
    hn::CappedTag<float, 8> d;
    for (size_t lane = 0; lane < count; lane += hn::Lanes(d))
    {
        hn::Vec<decltype(d)> p[12];
        for (int i = 0; i < 12; ++i)
            p[i] = LoadBatch(d, positions + i * FmHighwayMaxLanes, lane, count);
        const auto& x0 = p[0]; const auto& y0 = p[1]; const auto& z0 = p[2];
        const auto& x1 = p[3]; const auto& y1 = p[4]; const auto& z1 = p[5];
        const auto& x2 = p[6]; const auto& y2 = p[7]; const auto& z2 = p[8];
        const auto& x3 = p[9]; const auto& y3 = p[10]; const auto& z3 = p[11];

        const auto x01 = x0 - x1, x02 = x0 - x2, x03 = x0 - x3;
        const auto x10 = hn::Neg(x01), x12 = x1 - x2, x13 = x1 - x3;
        const auto x20 = hn::Neg(x02), x21 = hn::Neg(x12), x23 = x2 - x3;
        const auto x30 = hn::Neg(x03), x31 = hn::Neg(x13), x32 = hn::Neg(x23);
        const auto y01 = y0 - y1, y02 = y0 - y2, y03 = y0 - y3;
        const auto y10 = hn::Neg(y01), y12 = y1 - y2, y13 = y1 - y3;
        const auto y20 = hn::Neg(y02), y21 = hn::Neg(y12), y23 = y2 - y3;
        const auto y30 = hn::Neg(y03), y31 = hn::Neg(y13), y32 = hn::Neg(y23);

        const auto det =
            (x10 * y2 + x02 * y1 + x21 * y0) * z3 +
            (x01 * y3 + x30 * y1 + x13 * y0) * z2 +
            (x20 * y3 + x03 * y2 + x32 * y0) * z1 +
            (x12 * y3 + x31 * y2 + x23 * y1) * z0;
        const auto invDet = hn::Set(d, 1.0f) / det;
        hn::Vec<decltype(d)> m[16] = {
            y12 * z3 + y31 * z2 + y23 * z1,
            y20 * z3 + y03 * z2 + y32 * z0,
            y01 * z3 + y30 * z1 + y13 * z0,
            y10 * z2 + y02 * z1 + y21 * z0,
            x21 * z3 + x13 * z2 + x32 * z1,
            x02 * z3 + x30 * z2 + x23 * z0,
            x10 * z3 + x03 * z1 + x31 * z0,
            x01 * z2 + x20 * z1 + x12 * z0,
            x12 * y3 + x31 * y2 + x23 * y1,
            x20 * y3 + x03 * y2 + x32 * y0,
            x01 * y3 + x30 * y1 + x13 * y0,
            x10 * y2 + x02 * y1 + x21 * y0,
            (x1 * y2 - x2 * y1) * z3 + (x3 * y1 - x1 * y3) * z2 + (x2 * y3 - x3 * y2) * z1,
            (x2 * y0 - x0 * y2) * z3 + (x0 * y3 - x3 * y0) * z2 + (x3 * y2 - x2 * y3) * z0,
            (x0 * y1 - x1 * y0) * z3 + (x3 * y0 - x0 * y3) * z1 + (x1 * y3 - x3 * y1) * z0,
            (x1 * y0 - x0 * y1) * z2 + (x0 * y2 - x2 * y0) * z1 + (x2 * y1 - x1 * y2) * z0
        };
        StoreBatch(d, det, determinant, lane, count);
        for (int i = 0; i < 16; ++i)
            StoreBatch(d, m[i] * invDet, barycentricMatrix + i * FmHighwayMaxLanes, lane, count);
    }
}

void FmHighwayComputeTetStrainImpl(
    const float* barycentricMatrix, const float* offsets, float* strain, size_t count)
{
    hn::CappedTag<float, 8> d;
    for (size_t lane = 0; lane < count; lane += hn::Lanes(d))
    {
        hn::Vec<decltype(d)> b[12], p[12];
        for (int i = 0; i < 12; ++i)
        {
            b[i] = LoadBatch(d, barycentricMatrix + i * FmHighwayMaxLanes, lane, count);
            p[i] = LoadBatch(d, offsets + i * FmHighwayMaxLanes, lane, count);
        }
        const auto sx = b[0] * p[0] + b[1] * p[3] + b[2] * p[6] + b[3] * p[9];
        const auto sy = b[4] * p[1] + b[5] * p[4] + b[6] * p[7] + b[7] * p[10];
        const auto sz = b[8] * p[2] + b[9] * p[5] + b[10] * p[8] + b[11] * p[11];
        const auto sxy = b[0] * p[1] + b[1] * p[4] + b[2] * p[7] + b[3] * p[10] +
                         b[4] * p[0] + b[5] * p[3] + b[6] * p[6] + b[7] * p[9];
        const auto syz = b[4] * p[2] + b[5] * p[5] + b[6] * p[8] + b[7] * p[11] +
                         b[8] * p[1] + b[9] * p[4] + b[10] * p[7] + b[11] * p[10];
        const auto sxz = b[0] * p[2] + b[1] * p[5] + b[2] * p[8] + b[3] * p[11] +
                         b[8] * p[0] + b[9] * p[3] + b[10] * p[6] + b[11] * p[9];
        hn::Vec<decltype(d)> values[6] = {sx, sy, sz, sxy, syz, sxz};
        for (int i = 0; i < 6; ++i)
            StoreBatch(d, values[i], strain + i * FmHighwayMaxLanes, lane, count);
    }
}

template <class D>
static HWY_INLINE void StoreCompare(D d, hn::Vec<D> lhs, hn::Vec<D> rhs,
    int op, std::uint8_t* output)
{
    hn::Vec<D> result;
    const auto one = hn::Set(d, hn::TFromD<D>(1));
    const auto zero = hn::Zero(d);
    switch (op)
    {
    case AMD::FmHwy::CompareLT: result = hn::IfThenElse(hn::Lt(lhs, rhs), one, zero); break;
    case AMD::FmHwy::CompareLE: result = hn::IfThenElse(hn::Le(lhs, rhs), one, zero); break;
    case AMD::FmHwy::CompareGT: result = hn::IfThenElse(hn::Gt(lhs, rhs), one, zero); break;
    case AMD::FmHwy::CompareGE: result = hn::IfThenElse(hn::Ge(lhs, rhs), one, zero); break;
    case AMD::FmHwy::CompareEQ: result = hn::IfThenElse(hn::Eq(lhs, rhs), one, zero); break;
    default: result = hn::IfThenElse(hn::Ne(lhs, rhs), one, zero); break;
    }
    HWY_ALIGN hn::TFromD<D> lanes[8];
    hn::Store(result, d, lanes);
    for (size_t i = 0; i < hn::Lanes(d); ++i)
        output[i] = lanes[i] != hn::TFromD<D>(0);
}

template <class D>
static void FloatUnaryImplN(const float* input, unsigned count, int op, float* output)
{
    D d;
    for (size_t lane = 0; lane < count; lane += hn::Lanes(d))
    {
        const auto value = hn::LoadU(d, input + lane);
        hn::Vec<D> result = value;
        switch (op)
        {
        case AMD::FmHwy::FloatNegate: result = hn::Neg(value); break;
        case AMD::FmHwy::FloatSqrt: result = hn::Sqrt(value); break;
        case AMD::FmHwy::FloatAbs: result = hn::Abs(value); break;
        case AMD::FmHwy::FloatAcos: result = hn::Acos(d, value); break;
        case AMD::FmHwy::FloatTan: result = hn::Tan(d, value); break;
        case AMD::FmHwy::FloatExp: result = hn::Exp(d, value); break;
        case AMD::FmHwy::FloatLog: result = hn::Log(d, value); break;
        }
        hn::StoreU(result, d, output + lane);
    }
}

template <class D>
static void FloatBinaryImplN(const float* lhs, const float* rhs, unsigned count,
    int op, float* output)
{
    D d;
    for (size_t lane = 0; lane < count; lane += hn::Lanes(d))
    {
        const auto a = hn::LoadU(d, lhs + lane);
        const auto b = hn::LoadU(d, rhs + lane);
        hn::Vec<D> result;
        switch (op)
        {
        case AMD::FmHwy::BinaryAdd: result = hn::Add(a, b); break;
        case AMD::FmHwy::BinarySub: result = hn::Sub(a, b); break;
        case AMD::FmHwy::BinaryMul: result = hn::Mul(a, b); break;
        case AMD::FmHwy::BinaryDiv: result = hn::Div(a, b); break;
        case AMD::FmHwy::BinaryMin: result = hn::Min(a, b); break;
        case AMD::FmHwy::BinaryMax: result = hn::Max(a, b); break;
        default: result = hn::Pow(d, a, b); break;
        }
        hn::StoreU(result, d, output + lane);
    }
}

template <class D, class T>
static void IntegerBinaryImplN(const T* lhs, const T* rhs, unsigned count,
    int op, T* output)
{
    D d;
    for (size_t lane = 0; lane < count; lane += hn::Lanes(d))
    {
        const auto a = hn::LoadU(d, lhs + lane);
        const auto b = hn::LoadU(d, rhs + lane);
        hn::Vec<D> result;
        switch (op)
        {
        case AMD::FmHwy::BinaryAdd: result = hn::Add(a, b); break;
        case AMD::FmHwy::BinarySub: result = hn::Sub(a, b); break;
        case AMD::FmHwy::BinaryMul: result = hn::Mul(a, b); break;
        case AMD::FmHwy::BinaryAnd: result = hn::And(a, b); break;
        case AMD::FmHwy::BinaryOr: result = hn::Or(a, b); break;
        default: result = hn::Xor(a, b); break;
        }
        hn::StoreU(result, d, output + lane);
    }
}

template <class D, class T>
static void CompareImplN(const T* lhs, const T* rhs, unsigned count,
    int op, std::uint8_t* output)
{
    D d;
    for (size_t lane = 0; lane < count; lane += hn::Lanes(d))
        StoreCompare(d, hn::LoadU(d, lhs + lane),
            hn::LoadU(d, rhs + lane), op, output + lane);
}

void FmHighwayFloatUnaryImpl(const float* input, unsigned count, int op, float* output)
{
    if (count == 4) FloatUnaryImplN<hn::CappedTag<float, 4> >(input, count, op, output);
    else FloatUnaryImplN<hn::CappedTag<float, 8> >(input, count, op, output);
}
void FmHighwayFloatBinaryImpl(const float* lhs, const float* rhs, unsigned count,
    int op, float* output)
{
    if (count == 4) FloatBinaryImplN<hn::CappedTag<float, 4> >(lhs, rhs, count, op, output);
    else FloatBinaryImplN<hn::CappedTag<float, 8> >(lhs, rhs, count, op, output);
}
void FmHighwayFloatCompareImpl(const float* lhs, const float* rhs, unsigned count,
    int op, std::uint8_t* output)
{
    if (count == 4) CompareImplN<hn::CappedTag<float, 4>, float>(lhs, rhs, count, op, output);
    else CompareImplN<hn::CappedTag<float, 8>, float>(lhs, rhs, count, op, output);
}
void FmHighwayIntBinaryImpl(const int32_t* lhs, const int32_t* rhs, unsigned count,
    int op, int32_t* output)
{
    if (count == 4) IntegerBinaryImplN<hn::CappedTag<int32_t, 4>, int32_t>(lhs, rhs, count, op, output);
    else IntegerBinaryImplN<hn::CappedTag<int32_t, 8>, int32_t>(lhs, rhs, count, op, output);
}
void FmHighwayIntCompareImpl(const int32_t* lhs, const int32_t* rhs, unsigned count,
    int op, std::uint8_t* output)
{
    if (count == 4) CompareImplN<hn::CappedTag<int32_t, 4>, int32_t>(lhs, rhs, count, op, output);
    else CompareImplN<hn::CappedTag<int32_t, 8>, int32_t>(lhs, rhs, count, op, output);
}
void FmHighwayUintBinaryImpl(const uint32_t* lhs, const uint32_t* rhs, unsigned count,
    int op, uint32_t* output)
{
    if (count == 4) IntegerBinaryImplN<hn::CappedTag<uint32_t, 4>, uint32_t>(lhs, rhs, count, op, output);
    else IntegerBinaryImplN<hn::CappedTag<uint32_t, 8>, uint32_t>(lhs, rhs, count, op, output);
}
void FmHighwayUintCompareImpl(const uint32_t* lhs, const uint32_t* rhs, unsigned count,
    int op, std::uint8_t* output)
{
    if (count == 4) CompareImplN<hn::CappedTag<uint32_t, 4>, uint32_t>(lhs, rhs, count, op, output);
    else CompareImplN<hn::CappedTag<uint32_t, 8>, uint32_t>(lhs, rhs, count, op, output);
}


void FmHighwayComputeTetStressAndStiffnessImpl(
    const float* barycentricMatrix, const float* volume,
    const float* youngsModulus, const float* poissonsRatio,
    float* stressMatrix, float* stiffnessMatrix, size_t count)
{
    hn::CappedTag<float, 8> d;
    for (size_t lane = 0; lane < count; lane += hn::Lanes(d))
    {
        hn::Vec<decltype(d)> a[4], b[4], c[4];
        for (int i = 0; i < 4; ++i)
        {
            a[i] = LoadBatch(d, barycentricMatrix + i * FmHighwayMaxLanes, lane, count);
            b[i] = LoadBatch(d, barycentricMatrix + (4 + i) * FmHighwayMaxLanes, lane, count);
            c[i] = LoadBatch(d, barycentricMatrix + (8 + i) * FmHighwayMaxLanes, lane, count);
        }
        const auto young = LoadBatch(d, youngsModulus, lane, count);
        const auto poisson = LoadBatch(d, poissonsRatio, lane, count);
        const auto vol = LoadBatch(d, volume, lane, count);
        const auto one = hn::Set(d, 1.0f);
        const auto scale = young / ((one + poisson) * (one - hn::Set(d, 2.0f) * poisson));
        const auto v0 = scale * (one - poisson);
        const auto v1 = scale * poisson;
        const auto v2 = scale * (hn::Set(d, 0.5f) - poisson);
        const auto zero = hn::Zero(d);

        for (int i = 0; i < 4; ++i)
        {
            hn::Vec<decltype(d)> normalStress[9] = {
                a[i] * v0, a[i] * v1, a[i] * v1,
                b[i] * v1, b[i] * v0, b[i] * v1,
                c[i] * v1, c[i] * v1, c[i] * v0};
            hn::Vec<decltype(d)> shearStress[9] = {
                b[i] * v2, zero, c[i] * v2,
                a[i] * v2, c[i] * v2, zero,
                zero, b[i] * v2, a[i] * v2};
            for (int e = 0; e < 9; ++e)
            {
                StoreBatch(d, normalStress[e], stressMatrix + (i * 9 + e) * FmHighwayMaxLanes, lane, count);
                StoreBatch(d, shearStress[e], stressMatrix + ((4 + i) * 9 + e) * FmHighwayMaxLanes, lane, count);
            }
        }

        int pairs[10][2] = {{0,0},{1,1},{2,2},{3,3},{0,1},{0,2},{0,3},{1,2},{1,3},{2,3}};
        for (int pair = 0; pair < 10; ++pair)
        {
            const int p = pairs[pair][0], q = pairs[pair][1];
            hn::Vec<decltype(d)> k[9] = {
                vol * (c[p] * c[q] * v2 + b[p] * b[q] * v2 + a[p] * a[q] * v0),
                vol * (a[q] * b[p] * v2 + a[p] * b[q] * v1),
                vol * (a[q] * c[p] * v2 + a[p] * c[q] * v1),
                vol * (a[p] * b[q] * v2 + a[q] * b[p] * v1),
                vol * (c[p] * c[q] * v2 + a[p] * a[q] * v2 + b[p] * b[q] * v0),
                vol * (b[q] * c[p] * v2 + b[p] * c[q] * v1),
                vol * (a[p] * c[q] * v2 + a[q] * c[p] * v1),
                vol * (b[p] * c[q] * v2 + b[q] * c[p] * v1),
                vol * (b[p] * b[q] * v2 + a[p] * a[q] * v2 + c[p] * c[q] * v0)
            };
            for (int e = 0; e < 9; ++e)
                StoreBatch(d, k[e], stiffnessMatrix + (pair * 9 + e) * FmHighwayMaxLanes, lane, count);
        }
    }
}

template <class D>
static HWY_INLINE hn::Vec<D> MulMatVecRow(
    const HMat3<D>& matrix, const hn::Vec<D> vector[3], int row)
{
    return matrix.e[row] * vector[0] +
        matrix.e[3 + row] * vector[1] +
        matrix.e[6 + row] * vector[2];
}

void FmHighwayComputeRotatedStiffnessImpl(
    const float* stiffnessMatrix, const float* restPositions,
    const float* rotation, float* rotatedStiffnessMatrix,
    float* restForce, size_t count)
{
    hn::CappedTag<float, 8> d;
    for (size_t lane = 0; lane < count; lane += hn::Lanes(d))
    {
        HMat3<decltype(d)> k[4][4];
        for (int i = 0; i < 4; ++i)
            k[i][i] = LoadMat3(d, stiffnessMatrix + i * 9 * FmHighwayMaxLanes, lane, count);
        k[1][0] = LoadMat3(d, stiffnessMatrix + 4 * 9 * FmHighwayMaxLanes, lane, count);
        k[2][0] = LoadMat3(d, stiffnessMatrix + 5 * 9 * FmHighwayMaxLanes, lane, count);
        k[3][0] = LoadMat3(d, stiffnessMatrix + 6 * 9 * FmHighwayMaxLanes, lane, count);
        k[2][1] = LoadMat3(d, stiffnessMatrix + 7 * 9 * FmHighwayMaxLanes, lane, count);
        k[3][1] = LoadMat3(d, stiffnessMatrix + 8 * 9 * FmHighwayMaxLanes, lane, count);
        k[3][2] = LoadMat3(d, stiffnessMatrix + 9 * 9 * FmHighwayMaxLanes, lane, count);
        k[0][1] = TransposeMat3(k[1][0]);
        k[0][2] = TransposeMat3(k[2][0]);
        k[0][3] = TransposeMat3(k[3][0]);
        k[1][2] = TransposeMat3(k[2][1]);
        k[1][3] = TransposeMat3(k[3][1]);
        k[2][3] = TransposeMat3(k[3][2]);

        const HMat3<decltype(d)> r = LoadMat3(d, rotation, lane, count);
        HMat3<decltype(d)> re[4][4];
        for (int row = 0; row < 4; ++row)
            for (int column = 0; column < 4; ++column)
                re[row][column] = MulMat3(d, r, k[row][column]);

        hn::Vec<decltype(d)> negativeRest[4][3];
        for (int point = 0; point < 4; ++point)
            for (int component = 0; component < 3; ++component)
                negativeRest[point][component] = hn::Neg(LoadBatch(
                    d, restPositions + (point * 3 + component) * FmHighwayMaxLanes,
                    lane, count));
        for (int row = 0; row < 4; ++row)
            for (int component = 0; component < 3; ++component)
            {
                hn::Vec<decltype(d)> sum = hn::Zero(d);
                for (int column = 0; column < 4; ++column)
                    sum += MulMatVecRow(re[row][column], negativeRest[column], component);
                StoreBatch(d, sum,
                    restForce + (row * 3 + component) * FmHighwayMaxLanes,
                    lane, count);
            }

        const HMat3<decltype(d)> inverseR = TransposeMat3(r);
        const HMat3<decltype(d)> outputOrder[10] = {
            MulMat3(d, re[0][0], inverseR), MulMat3(d, re[1][1], inverseR),
            MulMat3(d, re[2][2], inverseR), MulMat3(d, re[3][3], inverseR),
            MulMat3(d, re[1][0], inverseR), MulMat3(d, re[2][0], inverseR),
            MulMat3(d, re[3][0], inverseR), MulMat3(d, re[2][1], inverseR),
            MulMat3(d, re[3][1], inverseR), MulMat3(d, re[3][2], inverseR)};
        for (int matrix = 0; matrix < 10; ++matrix)
            StoreMat3(d, outputOrder[matrix],
                rotatedStiffnessMatrix + matrix * 9 * FmHighwayMaxLanes,
                lane, count);
    }
}

}  // namespace HWY_NAMESPACE
}  // namespace AMD
HWY_AFTER_NAMESPACE();

#if HWY_ONCE
namespace AMD {

HWY_EXPORT(FmHighwayTransposeAosToSoaImpl);
HWY_EXPORT(FmHighwayEigenSymm3x3Impl);
HWY_EXPORT(FmHighwaySvd3x3Impl);
HWY_EXPORT(FmHighwayComputeShapeParamsImpl);
HWY_EXPORT(FmHighwayComputeTetStrainImpl);
HWY_EXPORT(FmHighwayComputeTetStressAndStiffnessImpl);
HWY_EXPORT(FmHighwayComputeRotatedStiffnessImpl);
HWY_EXPORT(FmHighwayFloatUnaryImpl);
HWY_EXPORT(FmHighwayFloatBinaryImpl);
HWY_EXPORT(FmHighwayFloatCompareImpl);
HWY_EXPORT(FmHighwayIntBinaryImpl);
HWY_EXPORT(FmHighwayIntCompareImpl);
HWY_EXPORT(FmHighwayUintBinaryImpl);
HWY_EXPORT(FmHighwayUintCompareImpl);

void FmHighwayTransposeAosToSoa(size_t components, const float* aos, float* soa, size_t count)
{
    HWY_DYNAMIC_DISPATCH(FmHighwayTransposeAosToSoaImpl)(components, aos, soa, count);
}

void FmHighwayEigenSymm3x3(const float* matrix, float* eigenvalues, float* eigenvectors, size_t count)
{
    HWY_DYNAMIC_DISPATCH(FmHighwayEigenSymm3x3Impl)(matrix, eigenvalues, eigenvectors, count);
}

void FmHighwaySvd3x3(const float* matrix, float* u, float* singularValues, float* v,
    const float* tolerance, size_t count)
{
    HWY_DYNAMIC_DISPATCH(FmHighwaySvd3x3Impl)(matrix, u, singularValues, v, tolerance, count);
}

void FmHighwayComputeShapeParams(const float* positions, float* barycentricMatrix,
    float* determinant, size_t count)
{
    HWY_DYNAMIC_DISPATCH(FmHighwayComputeShapeParamsImpl)(
        positions, barycentricMatrix, determinant, count);
}

void FmHighwayComputeTetStrain(const float* barycentricMatrix, const float* offsets,
    float* strain, size_t count)
{
    HWY_DYNAMIC_DISPATCH(FmHighwayComputeTetStrainImpl)(
        barycentricMatrix, offsets, strain, count);
}

void FmHighwayComputeTetStressAndStiffness(const float* barycentricMatrix,
    const float* volume, const float* youngsModulus, const float* poissonsRatio,
    float* stressMatrix, float* stiffnessMatrix, size_t count)
{
    HWY_DYNAMIC_DISPATCH(FmHighwayComputeTetStressAndStiffnessImpl)(
        barycentricMatrix, volume, youngsModulus, poissonsRatio,
        stressMatrix, stiffnessMatrix, count);
}

void FmHighwayComputeRotatedStiffness(
    const float* stiffnessMatrix, const float* restPositions,
    const float* rotation, float* rotatedStiffnessMatrix,
    float* restForce, size_t count)
{
    HWY_DYNAMIC_DISPATCH(FmHighwayComputeRotatedStiffnessImpl)(
        stiffnessMatrix, restPositions, rotation,
        rotatedStiffnessMatrix, restForce, count);
}

namespace FmHwy {

void FloatUnary(const float* input, unsigned count, FloatOp op, float* output)
{
    HWY_DYNAMIC_DISPATCH(FmHighwayFloatUnaryImpl)(input, count, op, output);
}
void FloatBinary(const float* lhs, const float* rhs, unsigned count,
    BinaryOp op, float* output)
{
    HWY_DYNAMIC_DISPATCH(FmHighwayFloatBinaryImpl)(lhs, rhs, count, op, output);
}
void FloatCompare(const float* lhs, const float* rhs, unsigned count,
    CompareOp op, std::uint8_t* output)
{
    HWY_DYNAMIC_DISPATCH(FmHighwayFloatCompareImpl)(lhs, rhs, count, op, output);
}
void FloatSinCos(const float* input, unsigned count, float* s, float* c)
{
    for (unsigned i = 0; i < count; ++i)
    {
        s[i] = std::sin(input[i]);
        c[i] = std::cos(input[i]);
    }
}
void FloatSelect(const float* lhs, const float* rhs, const std::uint8_t* select,
    unsigned count, float* output)
{
    for (unsigned i = 0; i < count; ++i)
        output[i] = select[i] ? rhs[i] : lhs[i];
}
void IntBinary(const std::int32_t* lhs, const std::int32_t* rhs, unsigned count,
    BinaryOp op, std::int32_t* output)
{
    HWY_DYNAMIC_DISPATCH(FmHighwayIntBinaryImpl)(lhs, rhs, count, op, output);
}
void IntCompare(const std::int32_t* lhs, const std::int32_t* rhs, unsigned count,
    CompareOp op, std::uint8_t* output)
{
    HWY_DYNAMIC_DISPATCH(FmHighwayIntCompareImpl)(lhs, rhs, count, op, output);
}
void IntSelect(const std::int32_t* lhs, const std::int32_t* rhs,
    const std::uint8_t* select, unsigned count, std::int32_t* output)
{
    for (unsigned i = 0; i < count; ++i)
        output[i] = select[i] ? rhs[i] : lhs[i];
}
void UintBinary(const std::uint32_t* lhs, const std::uint32_t* rhs, unsigned count,
    BinaryOp op, std::uint32_t* output)
{
    HWY_DYNAMIC_DISPATCH(FmHighwayUintBinaryImpl)(lhs, rhs, count, op, output);
}
void UintCompare(const std::uint32_t* lhs, const std::uint32_t* rhs, unsigned count,
    CompareOp op, std::uint8_t* output)
{
    HWY_DYNAMIC_DISPATCH(FmHighwayUintCompareImpl)(lhs, rhs, count, op, output);
}
void UintSelect(const std::uint32_t* lhs, const std::uint32_t* rhs,
    const std::uint8_t* select, unsigned count, std::uint32_t* output)
{
    for (unsigned i = 0; i < count; ++i)
        output[i] = select[i] ? rhs[i] : lhs[i];
}
void BoolBinary(const std::uint8_t* lhs, const std::uint8_t* rhs, unsigned count,
    BinaryOp op, std::uint8_t* output)
{
    for (unsigned i = 0; i < count; ++i)
        output[i] = op == BinaryAnd ? lhs[i] & rhs[i] :
                    op == BinaryOr ? lhs[i] | rhs[i] : lhs[i] ^ rhs[i];
}
void BoolNot(const std::uint8_t* input, unsigned count, std::uint8_t* output)
{
    for (unsigned i = 0; i < count; ++i) output[i] = !input[i];
}
void BoolSelect(const std::uint8_t* lhs, const std::uint8_t* rhs,
    const std::uint8_t* select, unsigned count, std::uint8_t* output)
{
    for (unsigned i = 0; i < count; ++i)
        output[i] = select[i] ? rhs[i] : lhs[i];
}
bool BoolAll(const std::uint8_t* input, unsigned count)
{
    for (unsigned i = 0; i < count; ++i) if (!input[i]) return false;
    return true;
}
bool BoolAny(const std::uint8_t* input, unsigned count)
{
    for (unsigned i = 0; i < count; ++i) if (input[i]) return true;
    return false;
}

}  // namespace FmHwy
}  // namespace AMD
#endif
